//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_lmd_analyser.h"
#include "lomse_xml_parser.h"

#include <iostream>
#include <sstream>
//BUG: In my Ubuntu box next line causes problems since approx. 20/march/2011
#if (LOMSE_PLATFORM_WIN32 == 1)
    #include <locale>
#endif
#include <vector>
#include <algorithm>
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
#include "lomse_autobeamer.h"

using namespace std;

//helper functions
inline bool both_are_spaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }
inline bool is_tab(const char& c) { return c == '\t'; }
inline bool is_newline(const char& c) { return c == '\n'; }



namespace lomse
{

//---------------------------------------------------------------------------------------
// assign a tag to each valid XML element
enum ELmdTag
{
    k_tag_undefined = -1,

    k_tag_clef,
    k_tag_content,
    k_tag_color,
    k_tag_defineStyle,
    k_tag_dynamic,
    k_tag_group,
    k_tag_image,
    k_tag_instrument,
    k_tag_itemizedlist,
    k_tag_ldpmusic,
    k_tag_lenmusdoc,
    k_tag_link,
    k_tag_listitem,
    k_tag_musicData,
    k_tag_orderedlist,
    k_tag_para,
    k_tag_param,
    k_tag_parts,
    k_tag_score,
    k_tag_scorePlayer,
    k_tag_section,
    k_tag_styles,
    k_tag_table,
    k_tag_tableCell,
    k_tag_tableColumn,
    k_tag_tableBody,
    k_tag_tableHead,
    k_tag_tableRow,
    k_tag_txt,
};


//---------------------------------------------------------------------------------------
// The syntax analyser is based on the Interpreter pattern ()
//
// The basic idea is to have a class for each language symbol, terminal or nonterminal
// (classes derived from LmdElementAnalyser). The parse tree created by the Parser is an
// instance of the composite pattern and is traversed by the analysers to evaluate
// (interpret) the sentence.
//
// The result of this step (semantic analysis) is a decorated parse tree, that is, the
// parse tree with a ImoObj added to certain nodes. The ImoObj collects the information
// present on the subtree:
//      Input: parse tree.
//      Output: parse tree with ImoObjs
//


//---------------------------------------------------------------------------------------
// Abstract class: any element analyser must derive from it

class LmdElementAnalyser
{
protected:
    ostream& m_reporter;
    LmdAnalyser* m_pAnalyser;
    LibraryScope& m_libraryScope;
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    LmdElementAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor=nullptr)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor) {}
    virtual ~LmdElementAnalyser() {}
    ImoObj* analyse_node(XmlNode* pNode);

protected:

    //analysis
    virtual ImoObj* do_analysis() = 0;

    //error reporting
    bool error_missing_element(ELdpElement type);
    bool error_missing_element(const string& name);
    void report_msg(int numLine, const std::string& msg);
    void report_msg(int numLine, const std::stringstream& msg);
    void error_if_more_elements();
    void error_invalid_child();
    void error_msg(const string& msg);

    //helpers, to simplify writting grammar rules
    XmlNode m_analysedNode;
    XmlNode m_childToAnalyse;
    XmlNode m_nextParam;
    XmlNode m_nextNextParam;

    bool get_mandatory(ELdpElement type);
    bool get_mandatory(const string& name);
    void analyse_mandatory(ELdpElement type, ImoObj* pAnchor=nullptr);
    void analyse_mandatory(const string& name, ImoObj* pAnchor=nullptr);
    bool get_optional(ELdpElement type);
    bool get_optional(const string& name);
    bool analyse_optional(ELdpElement type, ImoObj* pAnchor=nullptr);
    bool analyse_optional(const string& name, ImoObj* pAnchor=nullptr);
    void analyse_one_or_more(ELdpElement* pValid, int nValid);
    void analyse_staffobjs_options(ImoStaffObj* pSO);
    void analyse_scoreobj_options(ImoScoreObj* pSO);
    inline ImoObj* analyse_child() { return m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr); }
    bool has_attribute(const string& name);
    string get_attribute(const string& name);
    string get_mandatory_string_attribute(const string& name, const string& sDefault,
                                          const string& element);
    string get_optional_string_attribute(const string& name, const string& sDefault);
    int get_attribute_as_integer(const string& name, int nNumber);

    //building the model
    void add_to_model(ImoObj* pImo);

    //auxiliary
    inline ImoId get_node_id() { return get_node_id(&m_analysedNode); }
    bool contains(ELdpElement type, ELdpElement* pValid, int nValid);
    inline const string& get_document_locator() {
        return m_pAnalyser->get_document_locator();
    }



    //-----------------------------------------------------------------------------------
    //XmlNode helper methods
    //-----------------------------------------------------------------------------------
    inline ImoId get_node_id(XmlNode* node) { return m_pAnalyser->get_node_id(node); }
    inline bool has_attribute(XmlNode* node, const string& name)
    {
        return node->attribute(name.c_str()) != nullptr;
    }
    inline ELdpElement get_type(XmlNode* node) { return m_pAnalyser->get_type(node); }

    inline bool is_type(XmlNode* node, ELdpElement type) { return get_type(node) == type; }
    float get_value_as_float(XmlNode* UNUSED(node))
    {
        //TODO_X
        return 0.0f;
    }

    //-----------------------------------------------------------------------------------
    // white space management
    //-----------------------------------------------------------------------------------
    string collapse_whitespace(string str)
    {
        //remove consecutive spaces leaving only one space
        string::iterator new_end = std::unique(str.begin(), str.end(), both_are_spaces);
        str.erase(new_end, str.end());
        return str;
    }

    string replace_tabs_by_spaces(string str)
    {
        //every tab (U+0009, '\t') is converted to a space (U+0020, ' ')
        std::replace_if(str.begin(), str.end(), is_tab, ' ');
        return str;
    }

    string remove_special_chars_before_linefeeds(string str)
    {
        //Each tab (U+0009, '\t'), carriage return (U+000D, '\r'), or space (U+0020, ' ')
        //character surrounding a linefeed (U+000A, '\n') character is removed

        //TODO implement remove_special_chars_before_linefeeds(string str)
        return str;
    }

    string replace_newlines_by_spaces(string str)
    {
        //Each newline (U+000A, '\n') is replaced by an space
        std::replace_if(str.begin(), str.end(), is_newline, ' ');
        return str;
    }

    string remove_newlines(string str)
    {
        //remove new line chars
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
        return str;
    }

    //-----------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------
    inline void post_event(SpEventInfo event)
    {
        m_libraryScope.post_event(event);
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
    void get_num_staff()
    {
        string staff = m_childToAnalyse.value();
        int nStaff;
        //http://www.codeguru.com/forum/showthread.php?t=231054
        std::istringstream iss(staff);
        if ((iss >> std::dec >> nStaff).fail())
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid staff '" + staff + "'. Replaced by '1'.");
            m_pAnalyser->set_current_staff(0);
        }
        else
            m_pAnalyser->set_current_staff(--nStaff);
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
    long get_long_value(long nDefault=0L)
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
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
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
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid yes/no value '" + value + "'. Replaced by default.");
            return nDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_string_value()
    {
        return m_childToAnalyse.value();
    }

    //-----------------------------------------------------------------------------------
    EHAlign get_alignment_value(EHAlign defaultValue)
    {
        const std::string& value = m_childToAnalyse.value();
        if (value == "left")
            return k_halign_left;
        else if (value == "right")
            return k_halign_right;
        else if (value == "center")
            return k_halign_center;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Invalid alignment value '" + value + "'. Assumed 'center'.");
            return defaultValue;
        }
    }

    //-----------------------------------------------------------------------------------
    Color get_color_child()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
        Color color;
        if (pImo->is_color_dto())
        {
            ImoColorDto* pColor = static_cast<ImoColorDto*>( pImo );
            color = pColor->get_color();
        }
        delete pImo;
        return color;
    }

    //-----------------------------------------------------------------------------------
    Color get_color_value()
    {
        string value = m_childToAnalyse.value();
        ImoColorDto* pColor = LOMSE_NEW ImoColorDto();
        pColor->set_from_string(value);
        if (!pColor->is_ok())
        {
            error_msg("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
            delete pColor;
            return Color(0,0,0);
        }
        Color color = pColor->get_color();
        delete pColor;
        return color;
    }

    //-----------------------------------------------------------------------------------
    float get_font_size_value()
    {
        const string value = m_childToAnalyse.value();
        int size = static_cast<int>(value.size()) - 2;
        string points = value.substr(0, size);
        float rNumber;
        std::istringstream iss(points);
        if ((iss >> std::dec >> rNumber).fail())
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid size '" + value + "'. Replaced by '12'.");
            return 12.0f;
        }
        else
            return rNumber;
    }

    //-----------------------------------------------------------------------------------
    ImoStyle* get_text_style_child(const string& defaulName="Default style")
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        string styleName = get_string_value();
        ImoStyle* pStyle = nullptr;

        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
        if (pScore)
        {
            pStyle = pScore->find_style(styleName);
            if (!pStyle)
            {
                //try to find it in document global styles
                Document* pDoc = m_pAnalyser->get_document_being_analysed();
                ImoDocument* pImoDoc = pDoc->get_im_root();
                if (pImoDoc)
                    pStyle = pImoDoc->find_style(styleName);
            }
            if (!pStyle)
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pScore->get_style_or_default(defaulName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    TPoint get_point_child()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
        TPoint point;
        if (pImo->is_point_dto())
        {
            ImoPointDto* pPoint = static_cast<ImoPointDto*>( pImo );
            point = pPoint->get_point();
        }
        delete pImo;
        return point;
    }

    //-----------------------------------------------------------------------------------
    TSize get_size_child()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
        TSize size;
        if (pImo->is_size_info())
        {
            ImoSizeDto* pSize = static_cast<ImoSizeDto*>( pImo );
            size = pSize->get_size();
        }
        delete pImo;
        return size;
    }

    //-----------------------------------------------------------------------------------
    float get_location_child()
    {
        return get_float_value(0.0f);
    }

    //-----------------------------------------------------------------------------------
    float get_width_child(float rDefault=1.0f)
    {
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_height_child(float rDefault=1.0f)
    {
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_float_child(float rDefault=1.0f)
    {
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_lenght_child(float rDefault=0.0f)
    {
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    ImoStyle* get_doc_text_style(const string& styleName)
    {
        ImoStyle* pStyle = nullptr;

        ImoDocument* pDoc = m_pAnalyser->get_root_imo_document();
        if (pDoc)
        {
            pStyle = pDoc->find_style(styleName);
            if (!pStyle)
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pDoc->get_style_or_default(styleName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    ELineStyle get_line_style_child()
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        const std::string& value = m_childToAnalyse.value();
        if (value == "none")
            return k_line_none;
        else if (value == "dot")
            return k_line_dot;
        else if (value == "solid")
            return k_line_solid;
        else if (value == "longDash")
            return k_line_long_dash;
        else if (value == "shortDash")
            return k_line_short_dash;
        else if (value == "dotDash")
            return k_line_dot_dash;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Element 'lineStyle': Invalid value '" + value
                + "'. Replaced by 'solid'." );
            return k_line_solid;
        }
    }

    //-----------------------------------------------------------------------------------
    ELineCap get_line_cap_child()
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        const std::string& value = m_childToAnalyse.value();
        if (value == "none")
            return k_cap_none;
        else if (value == "arrowhead")
            return k_cap_arrowhead;
        else if (value == "arrowtail")
            return k_cap_arrowtail;
        else if (value == "circle")
            return k_cap_circle;
        else if (value == "square")
            return k_cap_square;
        else if (value == "diamond")
            return k_cap_diamond;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Element 'lineCap': Invalid value '" + value
                + "'. Replaced by 'none'." );
            return k_cap_none;
        }
    }

    //-----------------------------------------------------------------------------------
    void check_visible(ImoInlinesContainer* pCO)
    {
        string value = m_childToAnalyse.value();
        if (value == "visible")
            pCO->set_visible(true);
        else if (value == "noVisible")
            pCO->set_visible(false);
        else
        {
            error_invalid_child();
            pCO->set_visible(true);
        }
    }

    //-----------------------------------------------------------------------------------
    NoteTypeAndDots get_note_type_and_dots()
    {
        string duration = m_childToAnalyse.value();
        NoteTypeAndDots figdots = ldp_duration_to_components(duration);
        if (figdots.noteType == k_unknown_notetype)
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown note/rest duration '" + duration + "'. Replaced by 'q'.");
            figdots.noteType = k_quarter;
        }
        return figdots;
    }

    //-----------------------------------------------------------------------------------
    void analyse_attachments(ImoStaffObj* pAnchor)
    {
        while( more_children_to_analyse() )
        {
            m_childToAnalyse = get_child_to_analyse();
            ELdpElement type = get_type(&m_childToAnalyse);
            if (is_auxobj(type))
                m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
            else
                error_invalid_child();

            move_to_next_child();
        }
    }

    //-----------------------------------------------------------------------------------
    bool is_auxobj(int type)
    {
        return     type == k_beam
                || type == k_text
                || type == k_textbox
                || type == k_line
                || type == k_fermata
                || type == k_tie
                || type == k_tuplet
                ;
    }

    //-----------------------------------------------------------------------------------
    ImoInlineLevelObj* analyse_inline_object()
    {
        // { <inlineWrapper> | <link> | <textItem> | <image> | <button> | <string> }

        if(more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            ELdpElement type = get_type(&m_childToAnalyse);
            if (   /*type == k_inlineWrapper
                ||*/ type == k_txt
                || type == k_image
                || type == k_link
               )
            {
                return static_cast<ImoInlineLevelObj*>(
                    m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr) );
            }
            else if (type == k_string)
            {
                //string: implicit <txt>
                return create_text_item( m_childToAnalyse.value() );
            }
            else
                error_invalid_child();

            move_to_next_child();
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    ImoTextItem* create_text_item(string value, ImoId id=k_no_imoid)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                    ImFactory::inject(k_imo_text_item, pDoc, id) );
        pText->set_text( collapse_whitespace( replace_newlines_by_spaces(value) ));
        return pText;
    }

    //-----------------------------------------------------------------------------------
    void analyse_optional_style(ImoContentObj* pParent)
    {
        // [<style>]
        ImoStyle* pStyle = nullptr;
        if (has_attribute("style"))
            pStyle = get_doc_text_style( get_attribute("style") );
        pParent->set_style(pStyle);
    }

    //-----------------------------------------------------------------------------------
    void analyse_inline_objects(ImoInlinesContainer* pParent)
    {
        // <inlineObject>*
        while( more_children_to_analyse() )
        {
            ImoInlineLevelObj* pItem = analyse_inline_object();
            if (pItem)
                pParent->add_item(pItem);

            move_to_next_child();
        }
    }

    //-----------------------------------------------------------------------------------
    void analyse_inline_or_block_objects(ImoBlocksContainer* pParent)
    {
        // {<inlineObject> | <blockObject>}*
        while (more_children_to_analyse())
        {
            m_childToAnalyse = get_child_to_analyse();
            ELdpElement type = get_type(&m_childToAnalyse);

            if (
               // inline: { <inlineWrapper> | <link> | <textItem> | <image> | <button> }
                /*type == k_inlineWrapper
                ||*/ type == k_txt
                || type == k_image
                || type == k_link
               // block:  { <list> | <para> | <score> | <table> }
                || type == k_itemizedlist
                || type == k_orderedlist
                || type == k_para
                || type == k_table
                || type == k_score
               )
            {
                m_pAnalyser->analyse_node(&m_childToAnalyse, pParent);
            }
            else if (type == k_string)
            {
                ImoTextItem* pText = create_text_item( m_childToAnalyse.value() );
                ImoObj* pSave = m_pAnchor;
                m_pAnchor = pParent;
                add_to_model(pText);
                m_pAnchor = pSave;
            }
            else
                error_invalid_child();

            move_to_next_child();
        }
    }

};

//---------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullLmdAnalyser : public LmdElementAnalyser
{
protected:
    const string m_tag;

public:
    NullLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    const string& tag)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope)
        , m_tag(tag)
        {
        }

    ImoObj* do_analysis()
    {
        cout << "Missing analyser for element '" << m_tag << "'. Node ignored." << endl;
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <anchorLine> = (anchorLine <destination-point>[<lineStyle>][<color>][<width>]
//@                            [<lineCapEnd>])
//@ <destination-point> = <location>    in Tenths
//@     i.e.: (anchorLine (dx value)(dy value)(lineStyle value)(color value)(width value))
//@

class AnchorLineLmdAnalyser : public LmdElementAnalyser
{
public:
    AnchorLineLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLineStyle* pLine = static_cast<ImoLineStyle*>(
                                    ImFactory::inject(k_imo_line_style, pDoc) );
        pLine->set_start_point( TPoint(0.0f, 0.0f) );
        pLine->set_start_edge(k_edge_normal);
        pLine->set_start_cap(k_cap_none);
        pLine->set_end_edge(k_edge_normal);

        // <destination-point> = <dx><dy>
        TPoint point;
        if (get_mandatory(k_dx))
            point.x = get_location_child();
        if (get_mandatory(k_dy))
            point.y = get_location_child();
        pLine->set_end_point( point );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            pLine->set_line_style( get_line_style_child() );

        // [<color>])
        if (get_optional(k_color))
            pLine->set_color( get_color_child() );

        // [<width>]
        if (get_optional(k_width))
            pLine->set_width( get_width_child(1.0f) );

        // [<lineCapEnd>])
        if (get_optional(k_lineCapEnd))
            pLine->set_end_cap( get_line_cap_child() );

        add_to_model( pLine );
        return pLine;
    }

};

//@--------------------------------------------------------------------------------------
//@ ImoBarline StaffObj
//@ <barline> = (barline) | (barline <type>[<visible>][<location>])
//@ <type> = label: { start | end | double | simple | startRepetition |
//@                   endRepetition | doubleRepetition }

class BarlineLmdAnalyser : public LmdElementAnalyser
{
public:
    BarlineLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc) );
        pBarline->set_type(k_barline_simple);

        // <type> (label)
        if (get_optional(k_label))
        {
            int type = get_barline_type();
            pBarline->set_type(type);
            if (type == k_barline_double_repetition || type == k_barline_end_repetition)
                pBarline->set_num_repeats(1);
        }

        // [<visible>][<location>]
        analyse_staffobjs_options(pBarline);

        error_if_more_elements();

        add_to_model(pBarline);
        return pBarline;
    }

protected:

    int get_barline_type()
    {
        string value = m_childToAnalyse.value();
        int type = k_barline_simple;
        if (value == "simple")
            type = k_barline_simple;
        else if (value == "double")
            type = k_barline_double;
        else if (value == "start")
            type = k_barline_start;
        else if (value == "end")
            type = k_barline_end;
        else if (value == "endRepetition")
            type = k_barline_end_repetition;
        else if (value == "startRepetition")
            type = k_barline_start_repetition;
        else if (value == "doubleRepetition")
            type = k_barline_double_repetition;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Unknown barline type '" + value + "'. 'simple' barline assumed.");
        }

        return type;
    }

};

//@--------------------------------------------------------------------------------------
//@ <beam> = (beam num <beamtype>)
//@ <beamtype> = label. Concatenation of 1 to 6 chars from set { + | - | = | f | b }
//@     meaning:
//@         +  begin
//@         =  continue
//@         -  end
//@         f  forward
//@         b  backward
//@                                 +     =+f   ==     --b
//@ Examples:                       ====================
//@     (beam 17 +)                 |     ==============
//@     (beam 17 =+f)               |     ===   |    ===
//@     (beam 17 ==)                |     |     |      |
//@     (beam 17 --b)              **    **    **     **


class BeamLmdAnalyser : public LmdElementAnalyser
{
public:
    BeamLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoBeamDto* pInfo = nullptr;

        // num
        if (get_optional(k_number))
            pInfo->set_beam_number( get_integer_value(0) );
        else
        {
            error_msg("Missing or invalid beam number. Beam ignored.");
            delete pInfo;
            return nullptr;
        }

        // <beamtype> (label)
        if (!get_optional(k_label) || !set_beam_type(pInfo))
        {
            error_msg("Missing or invalid beam type. Beam ignored.");
            delete pInfo;
            return nullptr;
        }

        return pInfo;
    }

protected:

    bool set_beam_type(ImoBeamDto* pInfo)
    {
        const std::string& value = m_childToAnalyse.value();
        if (value.size() < 7)
        {
            for (int i=0; i < int(value.size()); ++i)
            {
                if (value[i] == '+')
                    pInfo->set_beam_type(i, ImoBeam::k_begin);
                else if (value[i] == '=')
                    pInfo->set_beam_type(i, ImoBeam::k_continue);
                else if (value[i] == '-')
                    pInfo->set_beam_type(i, ImoBeam::k_end);
                else if (value[i] == 'f')
                    pInfo->set_beam_type(i, ImoBeam::k_forward);
                else if (value[i] == 'b')
                    pInfo->set_beam_type(i, ImoBeam::k_backward);
                else
                    return false;   //error
            }
            return true;   //ok
        }
        else
            return false;   //error
    }

};


//@--------------------------------------------------------------------------------------
//@ <bezier> = (bezier <bezier-location>* )
//@ <bezier-location> = { (start-x num) | (start-y num) | (end-x num) | (end-y num) |
//@                       (ctrol1-x num) | (ctrol1-y num) | (ctrol2-x num) | (ctrol2-y num) }
//@ <num> = real number, in tenths

class BezierLmdAnalyser : public LmdElementAnalyser
{
public:
    BezierLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBezierInfo* pBezier = static_cast<ImoBezierInfo*>(
                                    ImFactory::inject(k_imo_bezier_info, pDoc));

        while (more_children_to_analyse())
        {
            if (get_optional(k_start_x))
                set_x_in_point(ImoBezierInfo::k_start, pBezier);
            else if (get_optional(k_end_x))
                set_x_in_point(ImoBezierInfo::k_end, pBezier);
            else if (get_optional(k_ctrol1_x))
                set_x_in_point(ImoBezierInfo::k_ctrol1, pBezier);
            else if (get_optional(k_ctrol2_x))
                set_x_in_point(ImoBezierInfo::k_ctrol2, pBezier);
            else if (get_optional(k_start_y))
                set_y_in_point(ImoBezierInfo::k_start, pBezier);
            else if (get_optional(k_end_y))
                set_y_in_point(ImoBezierInfo::k_end, pBezier);
            else if (get_optional(k_ctrol1_y))
                set_y_in_point(ImoBezierInfo::k_ctrol1, pBezier);
            else if (get_optional(k_ctrol2_y))
                set_y_in_point(ImoBezierInfo::k_ctrol2, pBezier);
            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        add_to_model(pBezier);
        return pBezier;
    }

    void set_x_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        float value = get_float_value(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.x = value;
    }

    void set_y_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        float value = get_float_value(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.y = value;
    }
};

//@--------------------------------------------------------------------------------------
//@ <border> = (border <width><lineStyle><color>)
//@     i.e.: (border (width 2.5)(lineStyle solid)(color #ff0000))

class BorderLmdAnalyser : public LmdElementAnalyser
{
public:
    BorderLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoBorderDto* border = LOMSE_NEW ImoBorderDto();

        // <width>
        if (get_mandatory(k_width))
            border->set_width( get_width_child(1.0f) );

        // <lineStyle>
        if (get_mandatory(k_lineStyle))
            border->set_style( get_line_style_child() );

        // <color>
        if (get_mandatory(k_color))
            border->set_color( get_color_child() );

        add_to_model(border);
        return border;
    }

protected:

        void set_box_border(ImoTextBlockInfo& box)
        {
            ImoObj* pImo = analyse_child();
            if (pImo)
            {
                if (pImo->is_border_dto())
                    box.set_border( static_cast<ImoBorderDto*>(pImo) );
                delete pImo;
            }
        }

};

//@--------------------------------------------------------------------------------------
//@ <chord> = (chord <note>+ )

class ChordLmdAnalyser : public LmdElementAnalyser
{
public:
    ChordLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoChord* pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, pDoc) );

        // <note>*
        while (more_children_to_analyse())
        {
            if (!analyse_optional(k_note, pChord))
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        add_notes_to_music_data(pChord);
        return pChord;
    }

protected:

    void add_notes_to_music_data(ImoChord* pChord)
    {
        //get anchor musicData
        if (m_pAnchor && m_pAnchor->is_music_data())
        {
            ImoMusicData* pMD = static_cast<ImoMusicData*>(m_pAnchor);

            //add notes to musicData
            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes
                = pChord->get_related_objects();
            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
            for (it = notes.begin(); it != notes.end(); ++it)
            {
                ImoNote* pNote = static_cast<ImoNote*>( (*it).first );
                pMD->append_child_imo(pNote);
            }

            add_to_model(pChord);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT clef (type, symbolSize?, staff?, visible?, %Location;?) >
//
class ClefLmdAnalyser : public LmdElementAnalyser
{
public:
    ClefLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>(
                            ImFactory::inject(k_imo_clef, pDoc, get_node_id()) );

        // type
        if (get_optional("type"))
            pClef->set_clef_type( get_clef_type() );

        // symbolSize?
        if (get_optional(k_symbolSize))
            set_symbol_size(pClef);

        // staff?, visible?, %Location;?
        analyse_staffobjs_options(pClef);

        error_if_more_elements();

        //set values that can be inherited
        pClef->set_staff( m_pAnalyser->get_current_staff() );

        add_to_model(pClef);
        return pClef;
    }

 protected:

    int get_clef_type()
    {
        const std::string& value = m_childToAnalyse.value();
        int clef = LmdAnalyser::ldp_name_to_clef_type(value);
        if (clef == k_clef_undefined)
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Unknown clef type '" + value + "'. Assumed 'G'.");
            return k_clef_G2;
        }
        else
            return clef;
    }

    void set_symbol_size(ImoClef* pClef)
    {
        const std::string& value = m_childToAnalyse.first_child().value();
        if (value == "cue")
            pClef->set_symbol_size(k_size_cue);
        else if (value == "full")
            pClef->set_symbol_size(k_size_full);
        else if (value == "large")
            pClef->set_symbol_size(k_size_large);
        else
        {
            pClef->set_symbol_size(k_size_full);
            error_msg("Invalid symbol size '" + value + "'. 'full' size assumed.");
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <color> = (color <rgba>}
//@ <rgba> = label: { #rrggbb | #rrggbbaa }

class ColorLmdAnalyser : public LmdElementAnalyser
{
public:
    ColorLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope) {}

    ImoObj* do_analysis()
    {
        string value = m_analysedNode.value();
        ImoColorDto* pColor = LOMSE_NEW ImoColorDto();
        pColor->set_from_string(value);
        if (!pColor->is_ok())
        {
            error_msg("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
            delete pColor;
            return nullptr;
        }
        return pColor;
    }
};

//---------------------------------------------------------------------------------------
class ContentLmdAnalyser : public LmdElementAnalyser
{
public:
    ContentLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                                        ImFactory::inject(k_imo_content, pDoc) );

        //node must be added to model before adding content to it, because
        //dynamic objects will generate requests to create other obejcts
        //on the fly, and this requires that all nodes are properly chained
        //in the tree.
        add_to_model(pContent);

        while (more_children_to_analyse())
        {
            if (!(  analyse_optional("section", pContent)
                 || analyse_optional("para", pContent)
                 || analyse_optional("itemizedlist", pContent)
                 || analyse_optional("orderedlist", pContent)
                 || analyse_optional("table", pContent)
                 || analyse_optional("image", pContent)
                 || analyse_optional("score", pContent)
                 || analyse_optional("ldpmusic", pContent)
                 //|| analyse_optional("xmlmusic", pContent)
                 || analyse_optional("content", pContent)
                 || analyse_optional("dynamic", pContent)
                 || analyse_optional("scorePlayer", pContent)
               ))
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        error_if_more_elements();
        return pContent;
    }
};

//@--------------------------------------------------------------------------------------
//@ <newSystem> = (newSystem}

class ControlLmdAnalyser : public LmdElementAnalyser
{
public:
    ControlLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemBreak* pCtrl = static_cast<ImoSystemBreak*>(
                                    ImFactory::inject(k_imo_system_break, pDoc) );
        add_to_model(pCtrl);
        return pCtrl;
    }
};

//@--------------------------------------------------------------------------------------
//@ <cursor> = (cursor <instrNumber><staffNumber><timePos><objID>)
//@ <instrNumber> = integer number (0..n-1)
//@ <staffNumber> = integer number (0..n-1)
//@ <timePos> = float number
//@ <objID> = integer number
//@

class CursorLmdAnalyser : public LmdElementAnalyser
{
public:
    CursorLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoCursorInfo* pCursor = static_cast<ImoCursorInfo*>(
                                    ImFactory::inject(k_imo_cursor_info, pDoc));

        // <instrNumber>
        if (get_mandatory(k_number))
            pCursor->set_instrument( get_integer_value(0) );

        // <staffNumber>
        if (get_mandatory(k_number))
            pCursor->set_staff( get_integer_value(0) );

        // <timePos>
        if (get_mandatory(k_number))
            pCursor->set_time( get_float_value(0.0f) );

        // <objID>
        if (get_mandatory(k_number))
            pCursor->set_id( get_long_value(k_no_imoid) );

        add_to_model(pCursor);
        return pCursor;
    }
};

//---------------------------------------------------------------------------------------
class DefineStyleLmdAnalyser : public LmdElementAnalyser
{
public:
    DefineStyleLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoStyle* pStyle;
        string name;

        //<name>
        if (get_mandatory(k_name))
            name = get_string_value();
        else
            return nullptr;
        string parent = (name == "Default style" ? "" : "Default style");

        pStyle = create_style(name, parent);

        bool fHasFontFile = false;
        while (more_children_to_analyse())
        {
            // color and background
            if (get_optional(k_color))
                pStyle->color( get_color_value() );
            else if (get_optional(k_background_color))
                pStyle->background_color( get_color_value() );

            // font
            else if (get_optional("font-file"))
            {
                pStyle->font_file( get_string_value() );
                fHasFontFile = true;
            }
            else if (get_optional(k_font_name))
            {
                pStyle->font_name( get_string_value() );
                if (!fHasFontFile)
                    pStyle->font_file("");
            }
            else if (get_optional(k_font_size))
                pStyle->font_size( get_font_size_value() );
            else if (get_optional(k_font_style))
                pStyle->font_style( get_font_style() );
            else if (get_optional(k_font_weight))
                pStyle->font_weight( get_font_weight() );

//                // border
//                else if (get_optional(k_border))
//                    pStyle->border( get_lenght_child() );
//                else if (get_optional(k_border_top))
//                    pStyle->border_top( get_lenght_child() );
//                else if (get_optional(k_border_right))
//                    pStyle->border_right( get_lenght_child() );
//                else if (get_optional(k_border_bottom))
//                    pStyle->border_bottom( get_lenght_child() );
//                else if (get_optional(k_border_left))
//                    pStyle->border_left( get_lenght_child() );

            // border width
            else if (get_optional(k_border_width))
                pStyle->border_width( get_lenght_child() );
            else if (get_optional(k_border_width_top))
                pStyle->border_width_top( get_lenght_child() );
            else if (get_optional(k_border_width_right))
                pStyle->border_width_right( get_lenght_child() );
            else if (get_optional(k_border_width_bottom))
                pStyle->border_width_bottom( get_lenght_child() );
            else if (get_optional(k_border_width_left))
                pStyle->border_width_left( get_lenght_child() );

            // margin
            else if (get_optional(k_margin))
                pStyle->margin( get_lenght_child() );
            else if (get_optional(k_margin_top))
                pStyle->margin_top( get_lenght_child() );
            else if (get_optional(k_margin_right))
                pStyle->margin_right( get_lenght_child() );
            else if (get_optional(k_margin_bottom))
                pStyle->margin_bottom( get_lenght_child() );
            else if (get_optional(k_margin_left))
                pStyle->margin_left( get_lenght_child() );

            // padding
            else if (get_optional(k_padding))
                pStyle->padding( get_lenght_child() );
            else if (get_optional(k_padding_top))
                pStyle->padding_top( get_lenght_child() );
            else if (get_optional(k_padding_right))
                pStyle->padding_right( get_lenght_child() );
            else if (get_optional(k_padding_bottom))
                pStyle->padding_bottom( get_lenght_child() );
            else if (get_optional(k_padding_left))
                pStyle->padding_left( get_lenght_child() );

            //text
            else if (get_optional(k_text_decoration))
                pStyle->text_decoration( get_text_decoration() );
            else if (get_optional(k_vertical_align))
                pStyle->vertical_align( get_valign() );
            else if (get_optional(k_text_align))
                pStyle->text_align( get_text_align() );
            else if (get_optional(k_line_height))
                pStyle->line_height( get_float_child(1.5f) );

            //size
            else if (get_optional(k_min_height))
                pStyle->min_height( get_lenght_child() );
            else if (get_optional(k_min_width))
                pStyle->min_width( get_lenght_child() );
            else if (get_optional(k_max_height))
                pStyle->max_height( get_lenght_child() );
            else if (get_optional(k_max_width))
                pStyle->max_width( get_lenght_child() );
            else if (get_optional(k_height))
                pStyle->height( get_lenght_child() );
            else if (get_optional(k_width))
                pStyle->width( get_lenght_child() );

            //table
            else if (get_optional(k_table_col_width))
                pStyle->table_col_width( get_lenght_child() );

            else
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        if (pStyle->get_name() != "Default style")
            add_to_model(pStyle);

        return pStyle;
    }

protected:

    int get_font_style()
    {
        const string value = get_string_value();
        if (value == "normal")
            return ImoStyle::k_font_style_normal;
        else if (value == "italic")
            return ImoStyle::k_font_style_italic;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown font-style '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_style_normal;
        }
    }

    int get_font_weight()
    {
        const string value = get_string_value();
        if (value == "normal")
            return ImoStyle::k_font_weight_normal;
        else if (value == "bold")
            return ImoStyle::k_font_weight_bold;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown font-weight '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_weight_normal;
        }
    }

    int get_text_decoration()
    {
        const string value = get_string_value();
        if (value == "none")
            return ImoStyle::k_decoration_none;
        else if (value == "underline")
            return ImoStyle::k_decoration_underline;
        else if (value == "overline")
            return ImoStyle::k_decoration_overline;
        else if (value == "line-through")
            return ImoStyle::k_decoration_line_through;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown text decoration value '" + value + "'. Replaced by 'none'.");
            return ImoStyle::k_decoration_none;
        }
    }

    int get_valign()
    {
        const string value = get_string_value();
        if (value == "baseline")
            return ImoStyle::k_valign_baseline;
        else if (value == "sub")
            return ImoStyle::k_valign_sub;
        else if (value == "super")
            return ImoStyle::k_valign_super;
        else if (value == "top")
            return ImoStyle::k_valign_top;
        else if (value == "text-top")
            return ImoStyle::k_valign_text_top;
        else if (value == "middle")
            return ImoStyle::k_valign_middle;
        else if (value == "bottom")
            return ImoStyle::k_valign_bottom;
        else if (value == "text-bottom")
            return ImoStyle::k_valign_text_bottom;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown vertical align '" + value + "'. Replaced by 'baseline'.");
            return ImoStyle::k_valign_baseline;
        }
    }

    int get_text_align()
    {
        const string value = get_string_value();
        if (value == "left")
            return ImoStyle::k_align_left;
        else if (value == "right")
            return ImoStyle::k_align_right;
        else if (value == "center")
            return ImoStyle::k_align_center;
        else if (value == "justify")
            return ImoStyle::k_align_justify;
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown text align '" + value + "'. Replaced by 'left'.");
            return ImoStyle::k_align_left;
        }
    }

    ImoStyle* create_style(const string& name, const string& parent)
    {
        ImoStyle* pDefault = nullptr;
        ImoStyles* pStyles = nullptr;
        if (m_pAnchor && m_pAnchor->is_styles())
        {
            pStyles = static_cast<ImoStyles*>(m_pAnchor);
            pDefault = pStyles->get_default_style();
        }

        if (name == "Default style")
        {
            return pDefault;
        }
        else
        {
            ImoStyle* pParent = pDefault;
            if (pStyles)
                pParent = pStyles->get_style_or_default(parent);

            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoStyle* pStyle = static_cast<ImoStyle*>(ImFactory::inject(k_imo_style, pDoc));
            pStyle->set_name(name);
            pStyle->set_parent_style(pParent);
            return pStyle;
        }
    }

};

//@--------------------------------------------------------------------------------------
// <dynamic> = (dynamic <classid> <param>*)
// <classid> = (classid <label>)
// <param> = (param <name><value>)
// <name> = <label>
// <value> = <string>
//
// Example:
//  (dynamic
//      (classid IdfyCadences) width="100%" height="300" border="0">
//      (param cadences "all")
//      (param cadence_buttons "terminal,transient")
//      (param mode "earTraining")
//  )
//

class DynamicLmdAnalyser : public LmdElementAnalyser
{
public:
    DynamicLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // <classid>
        if (!has_attribute("classid"))
        {
            error_msg("dynamic: missing mandatory attribute 'classid'. Element ignored.");
            return nullptr;
        }


        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamic* pDyn = static_cast<ImoDynamic*>(
                            ImFactory::inject(k_imo_dynamic, pDoc, get_node_id()) );
        pDyn->set_classid( get_attribute("classid") );

        // [<param>*]
        while (analyse_optional(k_parameter, pDyn));

        error_if_more_elements();

        //ImoDynamic must be included in model before asking to create its content
        add_to_model(pDyn);

        //ask user app to generate content for this dynamic object
        RequestDynamic request(pDoc, pDyn);
        m_libraryScope.post_request(&request);

        return pDyn;
    }
};

//@--------------------------------------------------------------------------------------
//@ <fermata> = (fermata <placement>[<componentOptions>*])
//@ <placement> = { above | below }

class FermataLmdAnalyser : public LmdElementAnalyser
{
public:
    FermataLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoFermata* pImo = static_cast<ImoFermata*>(
                                ImFactory::inject(k_imo_fermata, pDoc) );


        // <placement>
        if (get_mandatory(k_label))
            set_placement(pImo);

        // [<componentOptions>*]
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        add_to_model(pImo);
        return pImo;
    }

    void set_placement(ImoFermata* pImo)
    {
        string value = m_childToAnalyse.value();
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown fermata placement '" + value + "'. Replaced by 'above'.");
            pImo->set_placement(k_placement_above);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <figuredBass> = (figuredBass <figuredBassSymbols>[<parenthesis>][<fbline>]
//@                              [<componentOptions>*] )
//@ <parenthesis> = (parenthesis { yes | no })  default: no
//@
//@ <fbline> = (fbline <numLine> start <startPoint><endPoint><width><color>)
//@ <fbline> = (fbline num {start | stop} [<startPoint>][<endPoint>][<width>][<color>])

//@ <figuredBassSymbols> = an string.
//@        It is formed by concatenation of individual strings for each interval.
//@        Each interval string is separated by a blank space from the previous one.
//@        And it can be enclosed in parenthesis.
//@        Each interval string is a combination of prefix, number and suffix,
//@        such as  "#3", "5/", "(3)", "2+" or "#".
//@        Valid values for prefix and suffix are:
//@            prefix = { + | - | # | b | = | x | bb | ## }
//@            suffix = { + | - | # | b | = | x | bb | ## | / | \ }
//@
//@ examples:
//@
//@        b6              (figuredBass "b6 b")
//@        b
//@
//@        7 ________      (figuredBass "7 5 2" (fbline 15 start))
//@        5
//@        2
//@
//@        6               (figuredBass "6 (3)")
//@        (3)
//@

class FiguredBassLmdAnalyser : public LmdElementAnalyser
{
public:
    FiguredBassLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {

        // <figuredBassSymbols> (string)
        if (get_mandatory(k_string))
        {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoFiguredBassInfo info( get_string_value() );

            // [<parenthesis>]
            if (get_optional(k_parenthesis))
            {
                //TODO: Not yet necessary. Not implemented in LenMus
            }

            // [<fbline>]
            if (get_optional(k_fbline))
            {}

            // [<componentOptions>*]
            //analyse_scoreobj_options(dto);

            error_if_more_elements();

            ImoFiguredBass* pImo = LOMSE_NEW ImoFiguredBass(info);
            add_to_model(pImo);
            return pImo;
        }
        return nullptr;
    }
};

////    //get figured bass string and split it into components
////    std::string sData = GetNodeName(pNode->GetParameter(1));
////    ImoFiguredBassInfo oFBData(sData);
////    if (oFBData.get_error_msg() != "")
////    {
////        AnalysisError(pNode, oFBData.get_error_msg());
////        return (ImoFiguredBass*)nullptr;    //error
////    }
////
////    //initialize options with default values
////    //AWARE: There can be two fblines, one starting in this FB and another
////    //one ending in it.
////    int nFBL=0;     //index to next fbline
////    lmFBLineInfo* pFBLineInfo[2];
////    pFBLineInfo[0] = (lmFBLineInfo*)nullptr;
////    pFBLineInfo[1] = (lmFBLineInfo*)nullptr;
////
////    //get options: <parenthesis> & <fbline>
////    int iP;
////    for(iP=2; iP <= nNumParms; ++iP)
////    {
////        lmLDPNode* pX = pNode->GetParameter(iP);
////        std::string sName = GetNodeName(pX);
////        if (sName == "parenthesis")
////            ;   //TODO
////        else if (sName == "fbline")     //start/end of figured bass line
////        {
////            if (nFBL > 1)
////                AnalysisError(pX, "[Element '%s'. More than two 'fbline'. Ignored.",
////                            sElmName.wx_str() );
////            else
////                pFBLineInfo[nFBL++] = AnalyzeFBLine(pX, pVStaff);
////        }
////        else
////            AnalysisError(pX, "[Element '%s'. Invalid parameter '%s'. Ignored.",
////                          sElmName.c_str(), sName.wx_str() );
////    }
////
////    //analyze remaining optional parameters: <location>, <cursorPoint>
////	lmLDPOptionalTags oOptTags(this);
////	oOptTags.SetValid(lm_eTag_Location_x, lm_eTag_Location_y, -1);		//finish list with -1
////	lmLocation tPos = g_tDefaultPos;
////	oOptTags.AnalyzeCommonOptions(pNode, iP, pVStaff, nullptr, nullptr, &tPos);
////
////	//create the Figured Bass object
////    ImoFiguredBass* pFB = pVStaff->AddFiguredBass(&oFBData, nId);
////	pFB->SetUserLocation(tPos);
////
////    //save cursor data
////    if (m_fCursorData && m_nCursorObjID == nId)
////        m_pCursorSO = pFB;
////
////    //add FB line, if exists
////    for(int i=0; i < 2; i++)
////    {
////        if (pFBLineInfo[i])
////        {
////            if (pFBLineInfo[i]->fStart)
////            {
////                //start of FB line. Save the information
////                pFBLineInfo[i]->pFB = pFB;
////                m_PendingFBLines.push_back(pFBLineInfo[i]);
////            }
////            else
////            {
////                //end of FB line. Add it to the internal model
////                AddFBLine(pFB, pFBLineInfo[i]);
////            }
////        }
////    }
////
////    return pFB;       //no error
////}

//@--------------------------------------------------------------------------------------
//@ <font> = (font <font_name> <font_size> <font_style>)
//@ <font_name> = string   i.e. "Times New Roman", "Trebuchet"
//@ <font_size> = num      in points
//@ <font_style> = { "bold" | "normal" | "italic" | "bold-italic" }
//@
//@ Compatibility 1.5
//@     size is a number followed by 'pt'. i.e.: 12pt


class FontLmdAnalyser : public LmdElementAnalyser
{
public:
    FontLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoFontStyleDto* pFont = LOMSE_NEW ImoFontStyleDto();

        //<font_name> = string
        if (get_mandatory(k_string))
            pFont->name = get_string_value();

        //<font_size> = num
        if (get_optional(k_number))
            pFont->size = get_float_value(8.0f);
        //<font_size>. Compatibility 1.5
        else if (get_mandatory(k_label))
            pFont->size = get_font_size_value();

        //<font_style>
        if (get_mandatory(k_label))
            set_font_style_weight(pFont);

        //add font info to parent
        add_to_model(pFont);
        return pFont;
    }

    void set_font_style_weight(ImoFontStyleDto* pFont)
    {
        const string& value = m_childToAnalyse.value();
        if (value == "bold")
        {
            pFont->weight = ImoStyle::k_font_weight_bold;
            pFont->style = ImoStyle::k_font_style_normal;
        }
        else if (value == "normal")
        {
            pFont->weight = ImoStyle::k_font_weight_normal;
            pFont->style = ImoStyle::k_font_style_normal;
        }
        else if (value == "italic")
        {
            pFont->weight = ImoStyle::k_font_weight_normal;
            pFont->style = ImoStyle::k_font_style_italic;
        }
        else if (value == "bold-italic")
        {
            pFont->weight = ImoStyle::k_font_weight_bold;
            pFont->style = ImoStyle::k_font_style_italic;
        }
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown font style '" + value + "'. Replaced by 'normal'.");
            pFont->weight = ImoStyle::k_font_weight_normal;
            pFont->style = ImoStyle::k_font_style_normal;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <goBack> = (goBack <timeShift>)
//@ <goFwd> = (goFwd <timeShift>)
//@ <timeShift> = { start | end | <number> | <duration> }
//@
//@ the time shift can be:
//@   a) one of the tags 'start' and 'end': i.e. (goBack start) (goFwd end)
//@   b) a number: the amount of 256th notes to go forward or backwards
//@   c) a note/rest duration, i.e. 'e..'

class GoBackFwdLmdAnalyser : public LmdElementAnalyser
{
public:
    GoBackFwdLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                ImFactory::inject(k_imo_go_back_fwd, pDoc) );
        bool fFwd = is_type(&m_analysedNode, k_goFwd);
        pImo->set_forward(fFwd);

        // <duration> |start | end (label) or <number>
        if (get_optional(k_label))
        {
            string duration = m_childToAnalyse.value();
            if (duration == "start")
            {
                if (!fFwd)
                    pImo->set_to_start();
                else
                {
                    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Element 'goFwd' has an incoherent value: go forward to start?. Element ignored.");
                    delete pImo;
                    return nullptr;
                }
            }
            else if (duration == "end")
            {
                if (fFwd)
                    pImo->set_to_end();
                else
                {
                    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Element 'goBack' has an incoherent value: go backwards to end?. Element ignored.");
                    delete pImo;
                    return nullptr;
                }
            }
            else
            {
                NoteTypeAndDots figdots = ldp_duration_to_components(duration);
                if (figdots.noteType == k_unknown_notetype)
                {
                    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Unknown duration '" + duration + "'. Element ignored.");
                    delete pImo;
                    return nullptr;
                }
                else
                {
                    TimeUnits rTime = to_duration(figdots.noteType, figdots.dots);
                    pImo->set_time_shift(rTime);
                }
            }
        }
        else if (get_optional(k_number))
        {
            float rTime = get_value_as_float(&m_childToAnalyse);
            if (rTime < 0.0f)
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Negative value for element 'goFwd/goBack'. Element ignored.");
                delete pImo;
                return nullptr;
            }
            else
                pImo->set_time_shift(rTime);
        }
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Unknown duration '" + m_childToAnalyse.name() + "'. Element ignored.");
            delete pImo;
            return nullptr;
        }

        error_if_more_elements();

        add_to_model(pImo);
        return pImo;
    }
};

//@--------------------------------------------------------------------------------------
//@ DEPRECATED: Since v1.6 <graphic> element is only supported in backwards
//@             compatibility mode.
//@
//@ <graphic> = (graphic line <xStart><yStart><xEnd><yEnd>)
//@ <xStart>,<yStart>,<xEnd>,<yEnd> = number in tenths, relative to current pos
//@ line width is always 1 tenth
//@ colour is always black
//@
//@ Examples:
//@    (graphic line 30 0  80 -20)
//@    (graphic line 30 10 80 0)
//@
#if LOMSE_COMPATIBILITY_LDP_1_5

class GraphicLmdAnalyser : public LmdElementAnalyser
{
public:
    GraphicLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // "line"
        if (get_optional(k_label))
        {
            string value = m_childToAnalyse.value();
            if (value != "line")
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Unknown type '" + value + "'. Element 'graphic' ignored.");
                return nullptr;
            }
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>(
                                    ImFactory::inject(k_imo_score_line, pDoc));

        //xStart
        if (get_mandatory(k_number))
            pLine->set_x_start( get_float_value(0.0f) );

        //yStart
        if (get_mandatory(k_number))
            pLine->set_y_start( get_float_value(0.0f) );

        //xEnd
        if (get_mandatory(k_number))
            pLine->set_x_end( get_float_value(0.0f) );

        //yEnd
        if (get_mandatory(k_number))
            pLine->set_y_end( get_float_value(0.0f) );

        error_if_more_elements();

        pLine->set_start_cap(k_cap_arrowhead);
        add_to_model(pLine);
        return pLine;
    }

};
#endif  //LOMSE_COMPATIBILITY_LDP_1_5

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT group (firstInstr, lastInstr, grpName?, grpAbbrev?,
//@                  grpSymbol? joinBarlines?) >
//@
//@ <!ELEMENT firstInstr (#PCDATA) >
//@ <!ELEMENT lastInstr (#PCDATA) >
//@ <!ELEMENT groupName (#PCDATA) >
//@ <!ELEMENT grpAbbrev (#PCDATA) >
//@ <!ELEMENT grpSymbol (#PCDATA) >
//@ <grpSymbol> = (symbol {none | brace | bracket} )
//@ <!ELEMENT joinBarlines (#PCDATA) >
//@

class GroupLmdAnalyser : public LmdElementAnalyser
{
public:
    GroupLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();

        // firstInstr
        ImoInstrument* pFirstInstr = nullptr;
        if (get_mandatory("firstInstr"))
        {
            string partId = m_childToAnalyse.first_child().value();
            pFirstInstr = pScore->get_instrument(partId);
//            if (pFirstInstr == nullptr)
//            {
//                error_msg("");
//            }
        }
        else
            return nullptr;

        // lastInstr
        ImoInstrument* pLastInstr = nullptr;
        if (get_mandatory("lastInstr"))
        {
            string partId = m_childToAnalyse.first_child().value();
            pLastInstr = pScore->get_instrument(partId);
//            if (pFirstInstr == nullptr)
//            {
//                error_msg("");
//            }
        }
        else
            return nullptr;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                                    ImFactory::inject(k_imo_instr_group, pDoc));
        pGrp->add_instrument(pFirstInstr);
        pGrp->add_instrument(pLastInstr);

        // grpName?
        if (get_optional("grpName"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pGrp->set_name(pText);
        }

        // grpAbbrev?
        if (get_optional("grpAbbrev"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pGrp->set_abbrev(pText);
        }

        // grpSymbol?
        if (get_optional("grpSymbol"))
            set_symbol(pGrp);

//        // joinBarlines?
//        if (get_optional("joinBarlines"))
//            pGrp->set_join_barlines(ImoInstrGroup::k_standard);
//
        error_if_more_elements();

        add_to_model(pGrp);
        return pGrp;
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
            error_msg("Invalid value for <grpSymbol>. Must be 'none', 'brace', "
                      "'bracket' or 'line'. 'none' assumed.");
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
// <image [style='']>
//      <file>xxx</file>  <!-- name & ext. no path -->
// </image>

class ImageLmdAnalyser : public LmdElementAnalyser
{
public:
    ImageLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoImage* pImg = static_cast<ImoImage*>( ImFactory::inject(k_imo_image, pDoc) );

        // [<style>]
        analyse_optional_style(pImg);

        // <file>
        if (get_mandatory(k_file))
            load_image(pImg, m_childToAnalyse.value(), get_document_locator());

        add_to_model(pImg);
        return pImg;
    }

protected:

    void load_image(ImoImage* pImg, string imagename, string locator)
    {
        LmbDocLocator loc(locator);
        SpImage img = ImageReader::load_image( loc.get_locator_for_image(imagename) );
        pImg->set_content(img);
        if (!img->is_ok())
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode), "Error loading image. " + img->get_error_msg());
    }
};

//@--------------------------------------------------------------------------------------
//@ <infoMIDI> = (infoMIDI num_instr [num_channel])
//@ num_instr = integer: 0..255
//@ num_channel = integer: 0..15

class InfoMidiLmdAnalyser : public LmdElementAnalyser
{
public:
    InfoMidiLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                                    ImFactory::inject(k_imo_sound_info, pDoc) );


        // num_instr
        if (!get_optional(k_number) || !set_midi_program(pInfo))
        {
            error_msg("Missing or invalid MIDI instrument (0..127). MIDI info ignored.");
            delete pInfo;
            return nullptr;
        }

        // [num_channel]
        if (get_optional(k_number) && !set_midi_channel(pInfo))
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                        "Invalid MIDI channel (0..15). Channel info ignored.");
        }

        error_if_more_elements();

        add_to_model(pInfo);
        return pInfo;
    }

protected:

    bool set_midi_program(ImoSoundInfo* pInfo)
    {
        int value = get_integer_value(0);
        if (value < 0 || value > 127)
            return false;   //error

        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        pMidi->set_midi_program(value);
        return true;
    }

    bool set_midi_channel(ImoSoundInfo* pInfo)
    {
        int value = get_integer_value(0);
        if (value < 0 || value > 15)
            return false;   //error

        ImoMidiInfo* pMidi = pInfo->get_midi_info();
        pMidi->set_midi_channel(value);
        return true;
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT instrument (instrId?, instrName?, instrAbbrev?, staves?, staff*,
//@                       infoMIDI?, musicData) >
//@ <instrName> = <textString>
//@ <instrAbbrev> = <textString>

class InstrumentLmdAnalyser : public LmdElementAnalyser
{
public:
    InstrumentLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        m_pAnalyser->clear_pending_relations();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrument* pInstrument = nullptr;

        // [<instrId>]
        if (get_optional("instrId"))
        {
            string partId = m_childToAnalyse.first_child().value();
            ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
            pInstrument = pScore->get_instrument(partId);
            if (pInstrument == nullptr)
            {
                error_msg("'instrId' is not defined in <parts> element. Instrument ignored.");
                return nullptr;
            }
            pInstrument->set_instr_id( m_childToAnalyse.first_child().value() );
        }
        else if (m_pAnalyser->is_instr_id_required())
        {
            error_msg("instrument: missing 'instrId' element. Instrument ignored.");
            return nullptr;
        }
        else
            pInstrument = static_cast<ImoInstrument*>(
                            ImFactory::inject(k_imo_instrument, pDoc) );

        // instrName?
        if (get_optional("instrName"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pInstrument->set_name(pText);
        }

        // instrAbbrev?
        if (get_optional("instrAbbrev"))
        {
            ImoScoreText* pText = get_name_abbrev();
            if (pText)
                pInstrument->set_abbrev(pText);
        }

        // staves?
        if (get_optional("staves"))
            set_staves(pInstrument);

        // staff*
        while (analyse_optional("staff", pInstrument));

        // infoMIDI?
        analyse_optional("infoMIDI", pInstrument);

        // musicData
        analyse_mandatory("musicData", pInstrument);

        error_if_more_elements();

        if (!m_pAnalyser->is_instr_id_required())
        {
            add_to_model(pInstrument);
            add_sound_info_if_needed(pInstrument);
        }

        return pInstrument;
    }

protected:

    void set_staves(ImoInstrument* pInstrument)
    {
        string staves = m_childToAnalyse.value();
        int nStaves;
        std::istringstream iss(staves);
        if ((iss >> std::dec >> nStaves).fail())
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid value '" + staves + "' for staves. Replaced by 1.");
        }
        else
        {
            for(; nStaves > 1; --nStaves)
                pInstrument->add_staff();
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

    void add_sound_info_if_needed(ImoInstrument* pInstr)
    {
        if (pInstr->get_num_sounds() == 0)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                                        ImFactory::inject(k_imo_sound_info, pDoc) );
            pInstr->add_sound_info(pInfo);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT key type %staffobjOptions;* ) >
//@ <!ELEMENT type (#PCDATA)>
//@ type = { C | G | D | A | E | B | F+ | C+ | C- | G- | D- | A- |
//@          E- | B- | F | a | e | b | f+ | c+ | g+ | d+ | a+ | a- |
//@          e- | b- | f | c | g | d }

class KeySignatureLmdAnalyser : public LmdElementAnalyser
{
public:
    KeySignatureLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>(
                        ImFactory::inject(k_imo_key_signature, pDoc, get_node_id()) );

        // type
        if (get_optional(k_label))
            pKey->set_key_type( get_key_type() );

        // %staffobjOptions;*
        analyse_staffobjs_options(pKey);

        error_if_more_elements();

        add_to_model(pKey);
        return pKey;
    }

    int get_key_type()
    {
        string value = m_childToAnalyse.value();
        int type = LmdAnalyser::ldp_name_to_key_type(value);
        if (type == k_key_undefined)
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Unknown key '" + value + "'. Assumed 'C'.");
            type = k_key_C;
        }
        return type;
    }

};

//---------------------------------------------------------------------------------------
class LdpmusicLmdAnalyser : public LmdElementAnalyser
{
public:
    LdpmusicLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        string src = m_analysedNode.value();
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        LdpParser parser(m_reporter, m_libraryScope.ldp_factory());
        parser.parse_text(src);
        LdpTree* tree = parser.get_ldp_tree();
        LdpAnalyser a(m_reporter, m_libraryScope, pDoc);
        ImoObj* pScore =  a.analyse_tree_and_get_object(tree);
        delete tree->get_root();
        m_pAnalyser->score_analysis_begin( static_cast<ImoScore*>(pScore) );

        add_to_model(pScore);
        m_pAnalyser->score_analysis_end();
        return pScore;
    }
};

//---------------------------------------------------------------------------------------
class LenmusdocLmdAnalyser : public LmdElementAnalyser
{
public:
    LenmusdocLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope) {}

    ImoObj* do_analysis()
    {
        ImoDocument* pImoDoc = nullptr;

        // <vers>
        string version = get_mandatory_string_attribute("vers", "0.0", "lenmusdoc");

        // <language>
        string language = get_optional_string_attribute("language", "en");

        //create document
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        pImoDoc = static_cast<ImoDocument*>(
                    ImFactory::inject(k_imo_document, pDoc, get_node_id()));
        pImoDoc->set_version(version);
        pImoDoc->set_language(language);
        m_pAnalyser->save_root_imo_document(pImoDoc);
        pDoc->set_imo_doc(pImoDoc);


        //// [<settings>]
        //analyse_optional(k_settings, pImoDoc);

        //// [<meta>]
        //analyse_optional(k_meta, pImoDoc);

        // [<styles>]
        if (!analyse_optional(k_styles, pImoDoc))
            add_default(pImoDoc);

        //// [<pageLayout>*]
        //while (analyse_optional(k_pageLayout, pImoDoc));

        // <content>
        analyse_mandatory(k_content, pImoDoc);

        error_if_more_elements();

        return pImoDoc;
    }

protected:

    string get_version()
    {
        return m_childToAnalyse.first_child().value();
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
//@ <line> = (line <startPoint><endPoint>[<width>][<color>]
//@                [<lineStyle>][<startCap>][<endCap>])
//@ <startPoint> = (startPoint <location>)      (coordinates in tenths)
//@ <endPoint> = (endPoint <location>)      (coordinates in tenths)
//@ <lineStyle> = (lineStyle { none | solid | longDash | shortDash | dot | dotDash } )
//@ <startCap> = (lineCapStart <capType>)
//@ <endCap> = (lineCapEnd <capType>)
//@ <capType> = label: { none | arrowhead | arrowtail | circle | square | diamond }
//@
//@ Example:
//@     (line (startPoint (dx 5.0)(dy 5.0)) (endPoint (dx 80.0)(dy -10.0))
//@           (width 1.0)(color #000000)(lineStyle solid)
//@           (lineCapStart arrowhead)(lineCapEnd none) )
//@

class LineLmdAnalyser : public LmdElementAnalyser
{
public:
    LineLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>(
                                    ImFactory::inject(k_imo_score_line, pDoc) );

        // <startPoint>
        if (get_mandatory(k_startPoint))
            pLine->set_start_point( get_point_child() );

        // <endPoint>
        if (get_mandatory(k_endPoint))
            pLine->set_end_point( get_point_child() );

        // [<width>]
        if (get_optional(k_width))
            pLine->set_width( get_width_child(1.0f) );

        // [<color>])
        if (get_optional(k_color))
            pLine->set_color( get_color_child() );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            pLine->set_line_style( get_line_style_child() );

        // [<startCap>]
        if (get_optional(k_lineCapStart))
            pLine->set_start_cap( get_line_cap_child() );

        // [<endCap>]
        if (get_optional(k_lineCapEnd))
            pLine->set_end_cap( get_line_cap_child() );

        add_to_model(pLine);
        return pLine;
    }

};

//@--------------------------------------------------------------------------------------
//@ <link> = (link [<style>] <url> <inlineObject>+ )
//@ <url> = (url <strings>)
//@
//@ Example:
//@     (link (url "#TheoryHarmony_ch3.lms")(txt "Harmony exercise"))
//@
//$
//$ Example:
//$     <link url='#TheoryHarmony_ch3.lms'>Harmony exercise</link>


class LinkLmdAnalyser : public LmdElementAnalyser
{
public:
    LinkLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLink* pLink = static_cast<ImoLink*>(
                               ImFactory::inject(k_imo_link, pDoc, get_node_id()) );


        // [<style>]
        analyse_optional_style(pLink);

        // <url>
        string url = get_mandatory_string_attribute("url", "", "link");
        pLink->set_url(url);

        // <inlineObject>+
        while( more_children_to_analyse() )
        {
            ImoInlineLevelObj* pItem = analyse_inline_object();
            if (pItem)
                pLink->add_item(pItem);

            move_to_next_child();
        }

        add_to_model(pLink);
        return pLink;
    }
};

//@--------------------------------------------------------------------------------------
//@ <list> = ("itemizedlist" | "orderedlist" [<style>] <listitem>* )
//@
class ListLmdAnalyser : public LmdElementAnalyser
{
public:
    ListLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        string type = m_analysedNode.name();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoList* pList = static_cast<ImoList*>(
                               ImFactory::inject(k_imo_list, pDoc, get_node_id()) );
        pList->set_list_type(type == "itemizedlist" ? ImoList::k_itemized
                                                    : ImoList::k_ordered);

        // [<style>]
        analyse_optional_style(pList);

        // <listitem>*
        while (analyse_optional(k_listitem, pList));
        error_if_more_elements();

        add_to_model(pList);
        return pList;
    }
};

//@--------------------------------------------------------------------------------------
//@ <listitem> = (listitem [<style>] {<inlineObject> | <blockObject>}* )
//@
class ListItemLmdAnalyser : public LmdElementAnalyser
{
public:
    ListItemLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoListItem* pListItem = static_cast<ImoListItem*>(
                            ImFactory::inject(k_imo_listitem, pDoc, get_node_id()) );
        // [<style>]
        analyse_optional_style(pListItem);

        // {<inlineObject> | <blockObject>}*
        analyse_inline_or_block_objects(pListItem);

        add_to_model(pListItem);
        return pListItem;
    }
};

//@--------------------------------------------------------------------------------------
//@ <metronome> = (metronome { <NoteType><TicksPerMinute> | <NoteType><NoteType> |
//@                            <TicksPerMinute> }
//@                          [parenthesis][<componentOptions>*] )
//@
//@ examples:
//@    (metronome q 80)                -->  quarter_note_sign = 80
//@    (metronome q q.)                -->  quarter_note_sign = dotted_quarter_note_sign
//@    (metronome 80)                  -->  m.m. = 80
//@    (metronome q 80 parenthesis)    -->  (quarter_note_sign = 80)
//@    (metronome 120 noVisible)       -->  nothing displayed

class MetronomeLmdAnalyser : public LmdElementAnalyser
{
public:
    MetronomeLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMetronomeMark* pMtr = static_cast<ImoMetronomeMark*>(
                                    ImFactory::inject(k_imo_metronome_mark, pDoc) );

        // { <NoteType><TicksPerMinute> | <NoteType><NoteType> | <TicksPerMinute> }
        if (get_optional(k_label))
        {
            NoteTypeAndDots figdots = get_note_type_and_dots();
            pMtr->set_left_note_type( figdots.noteType );
            pMtr->set_left_dots( figdots.dots );

            if (get_optional(k_number))
            {
                // case 1: <NoteType><TicksPerMinute>
                pMtr->set_ticks_per_minute( get_integer_value(60) );
                pMtr->set_mark_type(ImoMetronomeMark::k_note_value);
            }
            else if (get_optional(k_label))
            {
                // case 2: <NoteType><NoteType>
                NoteTypeAndDots figdots = get_note_type_and_dots();
                pMtr->set_right_note_type( figdots.noteType );
                pMtr->set_right_dots( figdots.dots );
                pMtr->set_mark_type(ImoMetronomeMark::k_note_note);
            }
            else
            {
                report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                        "Error in metronome parameters. Replaced by '(metronome 60)'.");
                pMtr->set_ticks_per_minute(60);
                pMtr->set_mark_type(ImoMetronomeMark::k_value);
                add_to_model(pMtr);
                return nullptr;
            }
        }
        else if (get_optional(k_number))
        {
            // case 3: <TicksPerMinute>
            pMtr->set_ticks_per_minute( get_integer_value(60) );
            pMtr->set_mark_type(ImoMetronomeMark::k_value);
        }
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                    "Missing metronome parameters. Replaced by '(metronome 60)'.");
            pMtr->set_ticks_per_minute(60);
            pMtr->set_mark_type(ImoMetronomeMark::k_value);
            add_to_model(pMtr);
            return nullptr;
        }

        // [parenthesis]
        if (get_optional(k_label))
        {
            if (m_childToAnalyse.value() == "parenthesis")
                pMtr->set_parenthesis(true);
            else
                error_invalid_child();
        }

        // [<componentOptions>*]
        analyse_scoreobj_options(pMtr);

        error_if_more_elements();

        add_to_model(pMtr);
        return pMtr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <musicData> = (musicData [{<note>|<rest>|<barline>|<chord>|<clef>|<figuredBass>|
//@                            <graphic>|<key>|<line>|<metronome>|<newSystem>|<spacer>|
//@                            <text>|<time>|<goFwd>|<goBack>}*] )
//@
//@ <graphic>, <line> and <text> elements are accepted for compatibility with 1.5.
//@ From 1.6 these elements will no longer be possible. They must go attached to an
//@ spacer or other staffobj


class MusicDataLmdAnalyser : public LmdElementAnalyser
{
public:
    MusicDataLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, pDoc) );

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_children_to_analyse())
        {
            if (! (analyse_optional(k_note, pMD)
                   || analyse_optional(k_rest, pMD)
                   || analyse_optional(k_na, pMD)
                   || analyse_optional(k_chord, pMD)
                   || analyse_optional(k_barline, pMD)
                   || analyse_optional(k_clef, pMD)
                   || analyse_optional(k_figuredBass, pMD)
                   || analyse_optional(k_key_signature, pMD)
                   || analyse_optional(k_metronome, pMD)
                   || analyse_optional(k_newSystem, pMD)
                   || analyse_optional(k_spacer, pMD)
                   || analyse_optional(k_time_signature, pMD)
                   || analyse_optional(k_goFwd, pMD)
                   || analyse_optional(k_goBack, pMD)
#if LOMSE_COMPATIBILITY_LDP_1_5
                   || analyse_optional(k_graphic, pMD)
                   || analyse_optional(k_line, pMD)
                   || analyse_optional(k_text, pMD)
#endif
                  ))
            {
                error_invalid_child();
                move_to_next_child();
            }
        }

        add_to_model(pMD);
        return pMD;
    }

};

//@--------------------------------------------------------------------------------------
//@ ImoNote, ImoRest StaffObj
//@ <note> = ({n | na} <pitch><duration> [<noteOptions>*][<noteRestOptions>*]
//@             [<componentOptions>*][<attachments>*])
//@ <rest> = (r <duration> [<noteRestOptions>*][<componentOptions>*][<attachments>*])
//@ <noteOptions> = { <tie> | <stem> | <slur> }
//@ <noteRestOptions> = { <beam> | <tuplet> | <voice> | <staffNum> | <fermata> }
//@ <pitch> = label
//@ <duration> = label
//@ <stem> = (stem { up | down })
//@ <voice> = (voice num)
//@ <staffNum> = (p num)

class NoteRestLmdAnalyser : public LmdElementAnalyser
{
protected:
    ImoTieDto* m_pTieDto;
    ImoTupletDto* m_pTupletInfo;
    ImoBeamDto* m_pBeamInfo;
    ImoSlurDto* m_pSlurDto;
    ImoFermata* m_pFermata;
    std::string m_srcOldBeam;
    std::string m_srcOldTuplet;

public:
    NoteRestLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pTieDto(nullptr)
        , m_pTupletInfo(nullptr)
        , m_pBeamInfo(nullptr)
        , m_pSlurDto(nullptr)
        , m_pFermata(nullptr)
        , m_srcOldBeam("")
        , m_srcOldTuplet("")
    {
    }

    ImoObj* do_analysis()
    {
        bool fIsRest = is_type(&m_analysedNode, k_rest);
        bool fInChord = !fIsRest && is_type(&m_analysedNode, k_na);

        // create object note or rest
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoNoteRest* pNR = nullptr;
        ImoNote* pNote = nullptr;
        ImoRest* pRest = nullptr;
        if (fIsRest)
        {
            pRest = static_cast<ImoRest*>(ImFactory::inject(k_imo_rest, pDoc));
            pNR = pRest;
        }
        else
        {
            pNote = static_cast<ImoNote*>(ImFactory::inject(k_imo_note, pDoc));
            pNR = pNote;
        }

        //pitch
        if (!fIsRest)
        {
            // <pitch> (label)
            if (get_mandatory(k_label))
                set_notated_pitch(pNote);
        }

        // <duration> (label)
        if (get_mandatory(k_label))
            set_duration(pNR);

        //compatibility 1.5
        //after duration we can find old abbreviated items (l, g, p, v) in any order
        bool fStartOldTie = false;
        bool fAddOldBeam = false;
        bool fAddOldTuplet = false;
        while(get_optional(k_label))
        {
            char firstChar = (m_childToAnalyse.value())[0];
            if (!fIsRest && firstChar == 'l')
                fStartOldTie = true;
            else if (firstChar == 'g')
            {
                fAddOldBeam = true;
                m_srcOldBeam = m_childToAnalyse.value();
            }
            else if (firstChar == 't')
            {
                fAddOldTuplet = true;
                m_srcOldTuplet = m_childToAnalyse.value();
            }
            else if (firstChar == 'v')
                get_voice();
            else if (firstChar == 'p')
                get_num_staff();
            else
                error_invalid_child();
        }
        pNR->set_staff( m_pAnalyser->get_current_staff() );
        pNR->set_voice( m_pAnalyser->get_current_voice() );


        if (!fIsRest)
        {
            // [<noteOptions>*] = [{ <tie> | <stem> | <slur> }*]
            while (more_children_to_analyse())
            {
                if (get_optional(k_tie))
                    m_pTieDto = static_cast<ImoTieDto*>( analyse_child() );
                else if (get_optional(k_stem))
                    set_stem(pNote);
                else if (get_optional(k_slur))
                    m_pSlurDto = static_cast<ImoSlurDto*>( analyse_child() );
                else
                    break;
            }
        }

        // [<noteRestOptions>*]
        analyse_note_rest_options(pNR);

        // [<componentOptions>*]
        analyse_scoreobj_options(pNR);

        add_to_model(pNR);

        // [<attachments>*]
        analyse_attachments(pNR);

        // add fermata
        if (m_pFermata)
            add_attachment(pNR, m_pFermata);

        //tie
        if (fStartOldTie)
            m_pAnalyser->start_old_tie(pNote, &m_childToAnalyse);
        else if (!fIsRest)
            m_pAnalyser->create_tie_if_old_syntax_tie_pending(pNote);

        add_tie_info(pNote);

        //tuplet
        if (fAddOldTuplet)
            set_old_tuplet(pNR);
        else if (m_pTupletInfo==nullptr && m_pAnalyser->is_tuplet_open())
            add_to_current_tuplet(pNR);

        add_tuplet_info(pNR);

        //beam
        if (fAddOldBeam)
            set_beam_g(pNR);
        else if (m_pBeamInfo==nullptr && m_pAnalyser->is_old_beam_open())
            add_to_old_beam(pNR);

        add_beam_info(pNR);

        //slur
        add_slur_info(pNote);

        //chord
        if (!fIsRest && fInChord)
        {
            ImoNote* pStartOfChordNote = m_pAnalyser->get_last_note();
            ImoChord* pChord;
            if (pStartOfChordNote->is_in_chord())
            {
                //chord already created. just add note to it
                pChord = pStartOfChordNote->get_chord();
            }
            else
            {
                //previous note is the base note. Create the chord
                pChord = static_cast<ImoChord*>(ImFactory::inject(k_imo_chord, pDoc));
                Document* pDoc = m_pAnalyser->get_document_being_analysed();
                pStartOfChordNote->include_in_relation(pDoc, pChord);
            }

            //add current note to chord
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            pNote->include_in_relation(pDoc, pChord);

        //TODO: check if note in chord has the same duration than base note
      //  if (fInChord && m_pLastNote
      //      && !IsEqualTime(m_pLastNote->GetDuration(), rDuration) )
      //  {
      //      report_msg("Error: note in chord has different duration than base note. Duration changed.");
		    //rDuration = m_pLastNote->GetDuration();
      //      nNoteType = m_pLastNote->GetNoteType();
      //      nDots = m_pLastNote->GetNumDots();
      //  }
        }

        //save this note as last note
        if (!fIsRest)
            m_pAnalyser->save_last_note(pNote);

        return pNR;
    }

protected:

    void analyse_note_rest_options(ImoNoteRest* pNR)
    {
        // { <beam> | <tuplet> | <voice> | <staffNum> | <fermata> }

        while( more_children_to_analyse() )
        {
            m_childToAnalyse = get_child_to_analyse();
            ELdpElement type = get_type(&m_childToAnalyse);
            if (type == k_tuplet)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
                m_pTupletInfo = static_cast<ImoTupletDto*>( pImo );
            }
            else if (type == k_fermata)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
                m_pFermata = static_cast<ImoFermata*>( pImo );
            }
            else if (type == k_beam)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(&m_childToAnalyse, nullptr);
                m_pBeamInfo = static_cast<ImoBeamDto*>( pImo );
            }
            else if (type == k_voice)
            {
                set_voice_element(pNR);
            }
            else if (type == k_staffNum)
            {
                set_staff_num_element(pNR);
            }
            else
                break;

            move_to_next_child();
        }
    }

    void set_notated_pitch(ImoNote* pNote)
    {
        string pitch = m_childToAnalyse.value();
        int step, octave;
        EAccidentals accidentals = k_no_accidentals;
        if (pitch == "*")
            pNote->set_notated_pitch(k_no_pitch, 4, k_no_accidentals);
        else
        {
            if (LmdAnalyser::ldp_pitch_to_components(pitch, &step, &octave, &accidentals))
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Unknown note pitch '" + pitch + "'. Replaced by 'c4'.");
                pNote->set_notated_pitch(k_step_C, 4, k_no_accidentals);
            }
            else
                pNote->set_notated_pitch(step, octave, accidentals);
        }
    }

    void set_duration(ImoNoteRest* pNR)
    {
        NoteTypeAndDots figdots = get_note_type_and_dots();
        pNR->set_note_type_and_dots(figdots.noteType, figdots.dots);
    }

    void set_beam_g(ImoNoteRest* pNR)
    {
        string type = m_srcOldBeam.substr(1);
        if (type == "+")
            start_g_beam(pNR);
        else if (type == "-")
            end_g_beam(pNR);
        else
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid parameter '" + m_childToAnalyse.value()
                + "'. Ignored.");
        }
    }

    void get_voice()
    {
        string voice = m_childToAnalyse.value().substr(1);
        int nVoice;
        std::istringstream iss(voice);
        if ((iss >> std::dec >> nVoice).fail())
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid voice 'v" + voice + "'. Replaced by 'v1'.");
            m_pAnalyser->set_current_voice(1);
        }
        else
            m_pAnalyser->set_current_voice(nVoice);
    }

    void set_stem(ImoNote* pNote)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        string value = get_string_value();
        if (value == "up")
            pNote->set_stem_direction(k_stem_up);
        else if (value == "down")
            pNote->set_stem_direction(k_stem_down);
        else
        {
            pNote->set_stem_direction(k_stem_default);
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                            "Invalid value '" + value
                            + "' for stem type. Default stem asigned.");
        }
    }

    void start_g_beam(ImoNoteRest* pNR)
    {
        int nNoteType = pNR->get_note_type();
        if (get_beaming_level(nNoteType) == -1)
            error_note_longer_than_eighth();
        else if (m_pAnalyser->is_old_beam_open())
        {
            error_beam_already_open();
            add_to_old_beam(pNR);
        }
        else
            add_to_old_beam(pNR);
    }

    void add_to_old_beam(ImoNoteRest* pNR)
    {
        ImoBeamDto* pInfo = LOMSE_NEW ImoBeamDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        m_pAnalyser->add_old_beam(pInfo);
    }

    void error_note_longer_than_eighth()
    {
        report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
            "Requesting beaming a note longer than eighth. Beam ignored.");
    }

    void error_beam_already_open()
    {
        report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
            "Requesting to start a beam (g+) but there is already an open beam. Beam ignored.");
    }

    void error_no_beam_open()
    {
        report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
            "Requesting to end a beam (g-) but there is no matching g+. Beam ignored.");
    }

    void end_g_beam(ImoNoteRest* pNR)
    {
        if (!m_pAnalyser->is_old_beam_open())
        {
            error_no_beam_open();
        }
        else
        {
            ImoBeamDto* pInfo = LOMSE_NEW ImoBeamDto();
            pInfo->set_note_rest(pNR);
            pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
            m_pAnalyser->close_old_beam(pInfo);
        }
    }

    int get_beaming_level(int nNoteType)
    {
        switch(nNoteType) {
            case k_eighth:
                return 0;
            case k_16th:
                return 1;
            case k_32nd:
                return 2;
            case k_64th:
                return 3;
            case k_128th:
                return 4;
            case k_256th:
                return 5;
            default:
                return -1; //Error: Requesting beaming a note longer than eight
        }
    }

    void add_to_current_tuplet(ImoNoteRest* pNR)
    {
        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_tuplet_type(ImoTupletDto::k_continue);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        m_pAnalyser->add_relation_info(pInfo);
    }

    void set_old_tuplet(ImoNoteRest* pNR)
    {
        string value = m_srcOldTuplet;
        bool fError = false;
        string sActual;
        string sNormal = "0";
        if (value.length() > 1)
        {
            if (value.length() == 2)
            {
                if (value[1] == '-')
                {
                    end_old_tuplet(pNR);
                    return;
                }
                else
                    sActual = value.substr(1);
            }
            else if (value.length() == 4 && value[2] == '/')
            {
                sActual = value.substr(1, 1);
                sNormal = value.substr(3, 1);
            }
            else
                fError = true;
        }
        else
            fError = true;

        locale loc;
        if (!fError && (!isdigit(sActual[0],loc) || !isdigit(sNormal[0],loc)))
            fError = true;

        if (fError)
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Invalid parameter '" + m_childToAnalyse.value()
                + "'. Ignored.");
        }
        else
        {
            if (m_pAnalyser->is_tuplet_open())
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Requesting to start a tuplet but there is already an open tuplet. Tuplet ignored.");
                add_to_current_tuplet(pNR);
            }
            else
            {
                int actual;
                int normal;
                stringstream(sActual) >> actual;
                stringstream(sNormal) >> normal;
                start_old_tuplet(pNR, actual, normal);
            }
        }
    }

    void end_old_tuplet(ImoNoteRest* pNR)
    {
        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_tuplet_type(ImoTupletDto::k_stop);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        m_pAnalyser->add_relation_info(pInfo);
    }

    void start_old_tuplet(ImoNoteRest* pNR, int actual, int normal)
    {
        if (normal == 0)
        {
            if (actual == 2)
                normal = 3;   //duplet
            else if (actual == 3)
                normal = 2;   //triplet
            else if (actual == 4)
                normal = 6;
            else if (actual == 5)
                normal = 6;
            //else
            //    pInfo->set_normal_number(0);  //required
        }
        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_actual_number(actual);
        pInfo->set_normal_number(normal);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
        m_pAnalyser->add_relation_info(pInfo);
    }

    void set_voice_element(ImoNoteRest* pNR)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        int voice = get_integer_value( m_pAnalyser->get_current_voice() );
        m_pAnalyser->set_current_voice(voice);
        pNR->set_voice(voice);
    }

    void set_staff_num_element(ImoNoteRest* pNR)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();
        int curStaff = m_pAnalyser->get_current_staff() + 1;
        int staff = get_integer_value(curStaff) - 1;
        m_pAnalyser->set_current_staff(staff);
        pNR->set_staff(staff);
    }

    void add_tie_info(ImoNote* pNote)
    {
        if (m_pTieDto)
        {
            m_pTieDto->set_note(pNote);
            m_pAnalyser->add_relation_info(m_pTieDto);
        }
    }

    void add_tuplet_info(ImoNoteRest* pNR)
    {
        if (m_pTupletInfo)
        {
            m_pTupletInfo->set_note_rest(pNR);
            m_pAnalyser->add_relation_info(m_pTupletInfo);
        }
    }

    void add_beam_info(ImoNoteRest* pNR)
    {
        if (m_pBeamInfo)
        {
            m_pBeamInfo->set_note_rest(pNR);
            m_pAnalyser->add_relation_info(m_pBeamInfo);
        }
    }

    void add_slur_info(ImoNote* pNR)
    {
        if (m_pSlurDto)
        {
            m_pSlurDto->set_note(pNR);
            m_pAnalyser->add_relation_info(m_pSlurDto);
        }
    }

    void add_attachment(ImoNoteRest* pNR, ImoFermata* pFermata)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        pNR->add_attachment(pDoc, pFermata);
    }

};

//@--------------------------------------------------------------------------------------
//@ <option> = (opt <name><value>)
//@ <name> = label
//@ <value> = { number | label | string }

class OptLmdAnalyser : public LmdElementAnalyser
{
public:
    OptLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoOptionInfo* pOpt = nullptr;

        // <name> (label)
        string name;
        if (get_mandatory(k_label))
            name = m_childToAnalyse.value();

        // <value> { number | label | string }
        if (get_optional(k_label) || get_optional(k_number) || get_optional(k_string))
        {
            pOpt =  set_option(name);
            if (pOpt)
                error_if_more_elements();
            else
            {
                if ( is_bool_option(name) || is_number_long_option(name)
                     || is_number_float_option(name) || is_string_option(name) )
                    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Invalid value for option '" + name + "'. Option ignored.");
                else
                    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Invalid option '" + name + "'. Option ignored.");
            }
        }
        else
            error_msg("Missing value for option '" + name + "'. Option ignored.");

        return pOpt;
    }


    ImoOptionInfo* set_option(string& name)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(
                                        ImFactory::inject(k_imo_option, pDoc) );
        pOpt->set_name(name);

        bool fOk = false;
        if (is_bool_option(name))
            fOk = set_bool_value(pOpt);
        else if (is_number_long_option(name))
            fOk= set_long_value(pOpt);
        else if (is_number_float_option(name))
            fOk = set_float_value(pOpt);
        else if (is_string_option(name))
            fOk = set_string_value(pOpt);

        //backwards compatibility for deprecated options
        if (fOk)
        {
            if (name == "Score.JustifyFinalBarline")
            {
                pOpt->set_name("Score.JustifyLastSystem");
                pOpt->set_type(ImoOptionInfo::k_number_long);
                if (pOpt->get_bool_value())
                    pOpt->set_long_value(1L);   //yes = 1-only if ends in barline
                else
                    pOpt->set_long_value(0L);   //no = 0-never justify last system
            }
            else if (name == "StaffLines.StopAtFinalBarline")
            {
                pOpt->set_name("StaffLines.Truncate");
                pOpt->set_type(ImoOptionInfo::k_number_long);
                if (pOpt->get_bool_value())
                    pOpt->set_long_value(1L);   //yes = 1-only barline of type final
                else
                    pOpt->set_long_value(0L);   //no = 0-never
            }
        }

        if (fOk)
        {
            add_to_model(pOpt);
            return pOpt;
        }
        else
            delete pOpt;

        return nullptr;
    }

    bool is_bool_option(const string& name)
    {
        return (name == "Score.FillPageWithEmptyStaves")
            || (name == "Score.JustifyFinalBarline")
            || (name == "StaffLines.StopAtFinalBarline")    //deprecated 2.1
            || (name == "StaffLines.Hide")
            || (name == "Staff.DrawLeftBarline");
    }

    bool is_number_long_option(const string& name)
    {
        return (name == "Render.SpacingMethod")
            || (name == "Render.SpacingOptions")
            || (name == "Score.JustifyLastSystem")
            || (name == "Staff.UpperLegerLines.Displacement")
            || (name == "StaffLines.Truncate")
            ;
    }

    bool is_number_float_option(const string& name)
    {
        return (name == "Render.SpacingFactor")
            || (name == "Render.SpacingFopt")
            || (name == "Render.SpacingValue")
            ;
    }

    bool is_string_option(const string& UNUSED(name))
    {
        return false;       //no options for now
    }

    bool set_bool_value(ImoOptionInfo* pOpt)
    {
        if (is_bool_value())
        {
            pOpt->set_bool_value( get_bool_value() );
            pOpt->set_type(ImoOptionInfo::k_boolean);
            return true;    //ok
        }
        return false;   //error
    }

    bool set_long_value(ImoOptionInfo* pOpt)
    {
        if (is_long_value())
        {
            pOpt->set_long_value( get_long_value() );
            pOpt->set_type(ImoOptionInfo::k_number_long);
            return true;    //ok
        }
        return false;   //error
    }

    bool set_float_value(ImoOptionInfo* pOpt)
    {
        if (is_float_value())
        {
            pOpt->set_float_value( get_float_value() );
            pOpt->set_type(ImoOptionInfo::k_number_float);
            return true;    //ok
        }
        return false;   //error
    }

    bool set_string_value(ImoOptionInfo* pOpt)
    {
        string value = m_childToAnalyse.value();
        pOpt->set_string_value( value );
        pOpt->set_type(ImoOptionInfo::k_string);
        return true;   //no error
    }

};

//@--------------------------------------------------------------------------------------
//@ <pageLayout> = (pageLayout <pageSize><pageMargins><pageOrientation>)
//@ <pageSize> = (pageSize width height)
//@ <pageMargins> = (pageMargins left top right bottom binding)
//@ <pageOrientation> = [ "portrait" | "landscape" ]
//@ width, height, left, top right, bottom, binding = <num> in LUnits

class PageLayoutLmdAnalyser : public LmdElementAnalyser
{
public:
    PageLayoutLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoPageInfo* pInfo = static_cast<ImoPageInfo*>(
                                        ImFactory::inject(k_imo_page_info, pDoc) );

        // <pageSize>
        analyse_mandatory(k_pageSize, pInfo);

        // <pageMargins>
        analyse_mandatory(k_pageMargins, pInfo);

        // [ "portrait" | "landscape" ]
        if (!get_mandatory(k_label) || !set_orientation(pInfo))
        {
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                    "Invalid orientation. Expected 'portrait' or 'landscape'."
                    " 'portrait' assumed.");
            pInfo->set_portrait(true);
        }

        error_if_more_elements();

        add_to_model(pInfo);
        return pInfo;
    }

protected:

    bool set_orientation(ImoPageInfo* pInfo)
    {
        // return true if ok
        string type = m_childToAnalyse.value();
        if (type == "portrait")
        {
            pInfo->set_portrait(true);
            return true;
        }
        else if (type == "landscape")
        {
            pInfo->set_portrait(false);
            return true;
        }
        return false;
    }

};

//@--------------------------------------------------------------------------------------
//@ <pageMargins> = (pageMargins left top right bottom binding)     LUnits

class PageMarginsLmdAnalyser : public LmdElementAnalyser
{
public:
    PageMarginsLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
        else
            return nullptr;         //what is this for?
            //pDto = &dto;

        //left
        if (get_mandatory(k_number))
            pDto->set_left_margin( get_float_value(2000.0f) );

        //top
        if (get_mandatory(k_number))
            pDto->set_top_margin( get_float_value(2000.0f) );

        //right
        if (get_mandatory(k_number))
            pDto->set_right_margin( get_float_value(1500.0f) );

        //bottom
        if (get_mandatory(k_number))
            pDto->set_bottom_margin( get_float_value(2000.0f) );

        //binding
        if (get_mandatory(k_number))
            pDto->set_binding_margin( get_float_value(0.0f) );

        return pDto;
    }

};

//@--------------------------------------------------------------------------------------
//@ <pageSize> = (pageSize width height)        LUnits

class PageSizeLmdAnalyser : public LmdElementAnalyser
{
public:
    PageSizeLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
        else
            return nullptr;     //what is this for?
            //pDto = &dto;

        //width
        if (get_mandatory(k_number))
            pDto->set_page_width( get_float_value(21000.0f) );

        //height
        if (get_mandatory(k_number))
            pDto->set_page_height( get_float_value(29700.0f) );

        return pDto;
    }
};

//@--------------------------------------------------------------------------------------
//@ <paragraph> = (para [<style>] <inlineObject>*)
//
//$ <para>
//$ <!ELEMENT p %inlineObject;>
//$ <!ATTLIST p %coreattrs;>
//$
//$ <!-- core attributes common to most elements
//$    id       document-wide unique id
//$    style    associated style info
//$ -->
//$ <!ENTITY % coreattrs
//$  "id          ID             #IMPLIED
//$   style       %StyleName;    #IMPLIED
//$   >


class ParagraphLmdAnalyser : public LmdElementAnalyser
{
public:
    ParagraphLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoParagraph* pPara = static_cast<ImoParagraph*>(
                                    ImFactory::inject(k_imo_para, pDoc, get_node_id()) );
        // [<style>]
        analyse_optional_style(pPara);

        // <inlineObject>*
        analyse_inline_objects(pPara);

        add_to_model(pPara);
        return pPara;
    }

};

//@--------------------------------------------------------------------------------------
// <param name='name'>value</param>

class ParamLmdAnalyser : public LmdElementAnalyser
{
public:
    ParamLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        if (!has_attribute("name"))
        {
            error_msg("Missing name for element 'param'. Element ignored.");
            return nullptr;
        }

        string name = get_attribute("name");
        string value = m_analysedNode.value();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoParamInfo* pParam = static_cast<ImoParamInfo*>(
                                    ImFactory::inject(k_imo_param_info, pDoc) );
        pParam->set_name(name);
        pParam->set_value(value);
        add_to_model(pParam);
        return pParam;
    }
};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT parts (instrId+, group*) >
//@
class PartsLmdAnalyser : public LmdElementAnalyser
{
public:
    PartsLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();

        // instrId+
        if (get_optional("instrId"))
        {
            add_instrument(pScore);
            m_pAnalyser->require_instr_id();
            while (get_optional("instrId"))
                add_instrument(pScore);

            // group*
            while (analyse_optional("group", pScore));
        }
        else
        {
            error_msg("parts: at least one <instrId> is required.");
        }
        error_if_more_elements();

        return nullptr;
    }

protected:

    void add_instrument(ImoScore* pScore)
    {
        string instrId = m_childToAnalyse.first_child().value();
        if (pScore->get_instrument(instrId) == nullptr)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                        ImFactory::inject(k_imo_instrument, pDoc) );
            pScore->add_instrument(pInstr, instrId);
        }
        else
        {
            error_msg("parts: duplicated <instrId> will be ignored.");
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <point> = (tag (dx value)(dy value))

class PointLmdAnalyser : public LmdElementAnalyser
{
public:
    PointLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoPointDto point;

        // <dx>
        if (get_mandatory(k_dx))
            point.set_x( get_location_child() );

        // <dy>
        if (get_mandatory(k_dy))
            point.set_y( get_location_child() );

        error_if_more_elements();

        ImoPointDto* pImo = LOMSE_NEW ImoPointDto(point);
        add_to_model(pImo);
        return pImo;
    }
};

//@--------------------------------------------------------------------------------------
class SectionLmdAnalyser : public LmdElementAnalyser
{
public:
    SectionLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // <level> (num)
        int level;
        if (has_attribute("level"))
            level = get_attribute_as_integer("level", 1);
        else
        {
            error_msg("section: missing 'level' attribute. Level 1 assumed.");
            level = 1;
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoHeading* pHeading = static_cast<ImoHeading*>(
                        ImFactory::inject(k_imo_heading, pDoc, get_node_id()) );
        pHeading->set_level(level);

        // [<style>]
        analyse_optional_style(pHeading);

        // <inlineObject>+
        analyse_inline_objects(pHeading);

        add_to_model(pHeading);
        return pHeading;
    }

};

//@--------------------------------------------------------------------------------------
//@ <settings> = (settings [<cursor>][<undoData>])

class SettingsLmdAnalyser : public LmdElementAnalyser
{
public:
    SettingsLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //TODO all

        // [<cursor>]
        analyse_optional(k_cursor, m_pAnchor);

        // [<undoData>]
        analyse_optional(k_undoData, m_pAnchor);

        error_if_more_elements();

        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <!ELEMENT score (language?, defineStyle*, pageLayout*,
//@                  systemLayout*, option*, parts?, instrument+ ) >


class ScoreLmdAnalyser : public LmdElementAnalyser
{
public:
    ScoreLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, pDoc));
        m_pAnalyser->score_analysis_begin(pScore);
        m_pAnalyser->set_score_version("2.0");

//        // <vers>
//        if (get_mandatory(k_vers))
//        {
//            string version = get_version();
//            pScore->set_version(vers);
//        }

        // [<language>]
        analyse_optional(k_language);

//        // [<style>]
//        analyse_optional_style(pScore);

        // [<undoData>]
        //TODO: Not implemented in LenMus. Postponed until need is confirmed
        //analyse_optional(k_undoData, pScore);

        // [<creationMode>]
        //analyse_optional(k_creationMode, pScore);

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pScore));
        pScore->add_required_text_styles();

        // [<title>*]
        //while (analyse_optional(k_title, pScore));

        // [<pageLayout>*]
        while (analyse_optional(k_pageLayout, pScore));

        // [<systemLayout>*]
        while (analyse_optional(k_systemLayout, pScore));

        // [<cursor>]
        // Obsolete since 1.6, as cursor is now a document attribute
//        if (get_optional(k_cursor))
//            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
//                    "'cursor' in score is obsolete. Now must be in 'lenmusdoc' element. Ignored.");

        // [<option>*]
        while (analyse_optional(k_opt, pScore));

        // [<parts>]
        analyse_optional("parts", pScore);

        // <instrument>+
        if (!more_children_to_analyse())
            error_missing_element(k_instrument);
        else
        {
            while (more_children_to_analyse())
            {
                if (! analyse_optional(k_instrument, pScore))
                {
                    error_invalid_child();
                    move_to_next_child();
                }
            }
        }

        error_if_more_elements();

        add_to_model(pScore);
        m_pAnalyser->score_analysis_end();
        return pScore;
    }

protected:

    string get_version()
    {
        return m_childToAnalyse.first_child().value();
    }

//    void set_version(ImoScore* pScore)
//    {
//        string vers = m_pParamToAnalyse->get_parameter(1)->get_value();
//
//        //check that version is valid
//        if (vers == "1.5" || vers == "1.6" || vers == "1.7" || vers == "2.0")
//        {
//            //1.5 -
//            //1.6 -
//            //1.7 - transitional to 2.0. Was not published
//            //2.0
//        }
//        else if (vers == "2.1")
//        {
//            //2.1 - sep/2006
//        }
//        else
//        {
//            report_msg(m_pParamToAnalyse->get_line_number(),
//                "Invalid score version '" + vers + "'. Version 1.6 assumed.");
//            vers = "1.6";
//        }
//        int numvers = m_pAnalyser->set_score_version(vers);
//        pScore->set_version(numvers);
//    }
//
//    void set_defaults_for_this_score_version(ImoScore* pScore)
//    {
//        //set default options for each score version
//        string version = pScore->get_version_string();
//        if (version == "2.1")
//        {
//            ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFactor");
//            pOpt->set_float_value(0.35f);
//
//            pOpt = pScore->get_option("Render.SpacingOptions");
//            pOpt->set_long_value(k_render_opt_breaker_optimal
//                                 | k_render_opt_dmin_global);
//        }
//        else
//        {
//            //vers. 1.5, 1.6, 1.7, 2.0. Default values are ok
//        }
//    }

};

//---------------------------------------------------------------------------------------
class ScorePlayerLmdAnalyser : public LmdElementAnalyser
{
public:
    ScorePlayerLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScorePlayer* pSP = static_cast<ImoScorePlayer*>(
                        ImFactory::inject(k_imo_score_player, pDoc, get_node_id()) );
        ScorePlayerCtrl* pPlayer = LOMSE_NEW ScorePlayerCtrl(m_libraryScope, pSP, pDoc);
        pSP->attach_player(pPlayer);
        pSP->attach_score( m_pAnalyser->get_last_analysed_score() );

        // attr: [style]
        analyse_optional_style(pSP);

        // playLabel?
        if (get_optional(k_playLabel))
            pSP->set_play_label( get_string_value() );

        // stopLabel?
        if (get_optional(k_stopLabel))
            pSP->set_stop_label( get_string_value() );

        // mm?
        if (get_optional(k_mm))
            pSP->set_metronome_mm( get_integer_value(60) );

        error_if_more_elements();

        add_to_model(pSP);
        return pSP;
    }
};

//@--------------------------------------------------------------------------------------
//@ <size> = (size <width><height>)
//@ <width> = (width number)        value in LUnits
//@ <height> = (height number)      value in LUnits
//@     i.e.; (size (width 160)(height 100.7))

class SizeLmdAnalyser : public LmdElementAnalyser
{
public:
    SizeLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSizeDto size;

        // <width>
        if (get_mandatory(k_width))
            size.set_width( get_width_child() );

        // <height>
        if (get_mandatory(k_height))
            size.set_height( get_height_child() );

        error_if_more_elements();

        ImoSizeDto* pImo = LOMSE_NEW ImoSizeDto(size);
        add_to_model(pImo);
        return pImo;
    }
};


//@--------------------------------------------------------------------------------------
//@ <slur> = (slur num <slurType>[<bezier>][color] )   ;num = slur number. integer
//@ <slurType> = { start | continue | stop }
//@
//@ Example:
//@     (slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )
//@

class SlurLmdAnalyser : public LmdElementAnalyser
{
public:
    SlurLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSlurDto* pInfo = LOMSE_NEW ImoSlurDto();
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // num
        if (get_mandatory(k_number))
            pInfo->set_slur_number( get_integer_value(0) );

        // <slurType> (label)
        if (!get_mandatory(k_label) || !set_slur_type(pInfo))
        {
            error_msg("Missing or invalid slur type. Slur ignored.");
            delete pInfo;
            return nullptr;
        }

        // [<bezier>]
        analyse_optional(k_bezier, pInfo);

        // [<color>]
        if (get_optional(k_color))
            pInfo->set_color( get_color_child() );

        return pInfo;
    }

protected:

    bool set_slur_type(ImoSlurDto* pInfo)
    {
//        const std::string& value = m_childToAnalyse.value();
//        if (value == "start")
//            pInfo->set_slur_type(ImoSlurData::k_start);
//        else if (value == "stop")
//            pInfo->set_slur_type(ImoSlurData::k_stop);
//        else if (value == "continue")
//            pInfo->set_slur_type(ImoSlurData::k_continue);
//        else
//            return false;   //error
//        return true;    //ok
        const std::string& value = m_childToAnalyse.value();
        if (value == "start")
            pInfo->set_start(true);
        else if (value == "stop")
            pInfo->set_start(false);
        else
            return false;   //error
        return true;    //ok
    }
};

//@--------------------------------------------------------------------------------------
//@ ImoSpacer StaffObj
//@ <spacer> = (spacer <width>[<staffobjOptions>*][<attachments>*])     width in Tenths

class SpacerLmdAnalyser : public LmdElementAnalyser
{
public:
    SpacerLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSpacer* pSpacer = static_cast<ImoSpacer*>(
                                    ImFactory::inject(k_imo_spacer, pDoc) );

        // <width>
        if (get_optional(k_number))
        {
            pSpacer->set_width( get_float_value() );
        }
        else
        {
            error_msg("Missing width for spacer. Spacer ignored.");
            delete pSpacer;
            return nullptr;
        }

        // [<staffobjOptions>*]
        analyse_staffobjs_options(pSpacer);

        add_to_model(pSpacer);

       // [<attachments>*]
        analyse_attachments(pSpacer);
        return pSpacer;
    }

};

//@--------------------------------------------------------------------------------------
//@ ImoStaffInfo SimpleObj
//@ <staff> = (staff <num> [<staffType>][<staffLines>][<staffSpacing>]
//@                        [<staffDistance>][<lineThickness>] )
//@
//@ <staffType> = (staffType { ossia | cue | editorial | regular | alternate } )
//@ <staffLines> = (staffLines <integer_num>)
//@ <staffSpacing> = (staffSpacing <real_num>)      LUnits (cents of millimeter)
//@ <staffDistance> = (staffDistance <real_num>)    LUnits (cents of millimeter)
//@ <lineThickness> = (lineThickness <real_num>)    LUnits (cents of millimeter)
//@
//@ Example:
//@     (staff 1 (staffType regular)(staffLines 5)(staffSpacing 180.00)
//@              (staffDistance 2000.00)(lineThickness 15.00))
//@

class StaffLmdAnalyser : public LmdElementAnalyser
{
public:
    StaffLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, pDoc) );

        // num_instr
        if (!get_optional(k_number) || !set_staff_number(pInfo))
        {
            error_msg("Missing or invalid staff number. Staff info ignored.");
            delete pInfo;
            return nullptr;
        }

        // [<staffType>]
        if (get_optional(k_staffType))
            set_staff_type(pInfo);

        // [<staffLines>]
        if (get_optional(k_staffLines))
            set_staff_lines(pInfo);

        // [<staffSpacing>]
        if (get_optional(k_staffSpacing))
        {
            m_childToAnalyse = m_childToAnalyse.first_child();
            pInfo->set_line_spacing( get_float_value(180.0f));
        }

        //[<staffDistance>]
        if (get_optional(k_staffDistance))
        {
            m_childToAnalyse = m_childToAnalyse.first_child();
            pInfo->set_staff_margin( get_float_value(1000.0f));
        }

        //[<lineThickness>]
        if (get_optional(k_lineThickness))
        {
            m_childToAnalyse = m_childToAnalyse.first_child();
            pInfo->set_line_thickness( get_float_value(15.0f));
        }

        error_if_more_elements();

        add_to_model(pInfo);
        return pInfo;
    }

protected:

    bool set_staff_number(ImoStaffInfo* pInfo)
    {
        int value = get_integer_value(0);
        if (value < 1)
            return false;   //error
        pInfo->set_staff_number(value-1);
        return true;
    }

    void set_staff_type(ImoStaffInfo* pInfo)
    {
        const std::string& value = m_childToAnalyse.first_child().value();
        if (value == "ossia")
            pInfo->set_staff_type(ImoStaffInfo::k_staff_ossia);
        else if (value == "cue")
            pInfo->set_staff_type(ImoStaffInfo::k_staff_cue);
        else if (value == "editorial")
            pInfo->set_staff_type(ImoStaffInfo::k_staff_editorial);
        else if (value == "regular")
            pInfo->set_staff_type(ImoStaffInfo::k_staff_regular);
        else if (value == "alternate")
            pInfo->set_staff_type(ImoStaffInfo::k_staff_alternate);
        else
            error_msg("Invalid staff type '" + value + "'. 'regular' staff assumed.");
    }

    void set_staff_lines(ImoStaffInfo* pInfo)
    {
        m_childToAnalyse = m_childToAnalyse.first_child();

        int value = get_integer_value(0);
        if (value < 1)
            error_msg("Invalid staff. Num lines must be greater than zero. Five assumed.");
        else
            pInfo->set_num_lines(value);
    }


};

//---------------------------------------------------------------------------------------
class StylesLmdAnalyser : public LmdElementAnalyser
{
public:
    StylesLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStyles* pStyles = static_cast<ImoStyles*>(ImFactory::inject(k_imo_styles, pDoc));

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pStyles));

        error_if_more_elements();

        add_to_model(pStyles);
        return pStyles;
    }

};

//@--------------------------------------------------------------------------------------
//@ <systemLayout> = (systemLayout {first | other} <systemMargins>)

class SystemLayoutLmdAnalyser : public LmdElementAnalyser
{
public:
    SystemLayoutLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemInfo* pInfo = static_cast<ImoSystemInfo*>(
                                        ImFactory::inject(k_imo_system_info, pDoc));

        // {first | other} <label>
        if (get_mandatory(k_label))
        {
            string type = m_childToAnalyse.value();
            if (type == "first")
                pInfo->set_first(true);
            else if (type == "other")
                pInfo->set_first(false);
            else
            {
                report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                        "Expected 'first' or 'other' value but found '" + type
                        + "'. 'first' assumed.");
                pInfo->set_first(true);
            }
        }

        // <systemMargins>
        analyse_mandatory(k_systemMargins, pInfo);

        error_if_more_elements();

        add_to_model(pInfo);
        return pInfo;
    }

};

//@--------------------------------------------------------------------------------------
//@ <systemMargins> = (systemMargins <leftMargin><rightMargin><systemDistance>
//@                                  <topSystemDistance>)
//@ <leftMargin>, <rightMargin>, <systemDistance>, <topSystemDistance> = number (Tenths)

class SystemMarginsLmdAnalyser : public LmdElementAnalyser
{
public:
    SystemMarginsLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoSystemInfo dto;
        ImoSystemInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_system_info())
            pDto = static_cast<ImoSystemInfo*>(m_pAnchor);
        else
            return nullptr;     //what is this for?
            //pDto = &dto;

        if (get_mandatory(k_number))
            pDto->set_left_margin(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_right_margin(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_system_distance(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_top_system_distance(get_float_value());

        error_if_more_elements();
        return pDto;
    }

};

//@--------------------------------------------------------------------------------------
//@ (table [<style>] [<tableColumn>*] [<tableHead>] <tableBody> )
//@

class TableLmdAnalyser : public LmdElementAnalyser
{
public:
    TableLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTable* pTable= static_cast<ImoTable*>(
                             ImFactory::inject(k_imo_table, pDoc, get_node_id()) );

        // [<style>]
        analyse_optional_style(pTable);

        // [<tableColumn>*]
        while( analyse_optional(k_table_column, pTable) );

        // [<tableHead>]
        analyse_optional(k_table_head, pTable);

        // <tableBody>
        analyse_mandatory(k_table_body, pTable);

        error_if_more_elements();

        add_to_model(pTable);
        return pTable;
    }

};

//@--------------------------------------------------------------------------------------
//@ <tableBody> = (tableBody <tableRow>* )
//@

class TableBodyLmdAnalyser : public LmdElementAnalyser
{
public:
    TableBodyLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableBody* pBody = static_cast<ImoTableBody*>(
                                ImFactory::inject(k_imo_table_body, pDoc, get_node_id()) );

        // <tableRow>*
        while( more_children_to_analyse() )
        {
            analyse_mandatory(k_table_row, pBody);
        }

        add_to_model(pBody);
        return pBody;
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableCell> = (tableCell [<style>] [<rowspan>] [<colspan>]
//@                          {<inlineObject> | <blockObject>}* )
//@ <rowspan> = (rowspan <num>)
//@ <colspan> = (colspan <num>)
//@
// <tableCell><rowspan>2</rowspan>This is a cell</tableCell>

class TableCellLmdAnalyser : public LmdElementAnalyser
{
public:
    TableCellLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableCell* pImo = static_cast<ImoTableCell*>(
                                ImFactory::inject(k_imo_table_cell, pDoc, get_node_id()) );
        //[<style>]
        analyse_optional_style(pImo);

        ////[<rowspan>]
        //if (has_attribute("rowspan"))
        //    pImo->set_rowspan( get_attribute_as_integer("rowspan", 1) );

        ////[<colspan>]
        //if (has_attribute("colspan"))
        //    pImo->set_colspan( get_attribute_as_integer("colspan", 1) );

        //[<rowspan>]
        if (get_optional("rowspan"))
            pImo->set_rowspan( get_integer_value(1) );

        //[<colspan>]
        if (get_optional("colspan"))
            pImo->set_colspan( get_integer_value(1) );

        // {<inlineObject> | <blockObject>}*
        analyse_inline_or_block_objects(pImo);

        error_if_more_elements();

        add_to_model(pImo);
        return pImo;
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableColumn> = (tableColumn <style>)
//@

class TableColumnLmdAnalyser : public LmdElementAnalyser
{
public:
    TableColumnLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // <style>
        if (has_attribute("style") && m_pAnchor->is_table())
        {
            ImoStyle* pStyle = get_doc_text_style( get_attribute("style") );
            ImoTable* pTable = static_cast<ImoTable*>(m_pAnchor);
            pTable->add_column_style(pStyle);
        }

        error_if_more_elements();
        return nullptr;
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableHead> = (tableHead <tableRow>* )
//@

class TableHeadLmdAnalyser : public LmdElementAnalyser
{
public:
    TableHeadLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableHead* pHead = static_cast<ImoTableHead*>(
                        ImFactory::inject(k_imo_table_head, pDoc, get_node_id()) );

        // <tableRow>*
        while( more_children_to_analyse() )
        {
            analyse_mandatory(k_table_row, pHead);
        }

        add_to_model(pHead);
        return pHead;
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableRow> = (tableRow [<style>] <tableCell>* )
//@

class TableRowLmdAnalyser : public LmdElementAnalyser
{
public:
    TableRowLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableRow* pRow = static_cast<ImoTableRow*>(
                        ImFactory::inject(k_imo_table_row, pDoc, get_node_id()) );

        // [<style>]
        analyse_optional_style(pRow);

        // <tableCell>*
        while( more_children_to_analyse() )
        {
            analyse_mandatory(k_table_cell, pRow);
        }

        add_to_model(pRow);
        return pRow;
    }
};

//@--------------------------------------------------------------------------------------
//@ <textbox> = (textbox <location><size>[<bgColor>][<border>]<text>[<anchorLine>])
//@ <location> = (dx value)(dy value)       values in Tenths, relative to cur.pos
//@     i.e.: (dx 50.0)(dy 5)
//@ <size> = (size <width><height>)
//@ <width> = (width number)        value in LUnits
//@ <height> = (height number)      value in LUnits
//@     i.e.; (size (width 160)(height 100.7))
//@ <border> = (border <width><lineStyle><color>)
//@     i.e.: (border (width 2.5)(lineStyle solid)(color #ff0000))
//@ <anchorLine> = (anchorLine <destination-point>[<lineStyle>][<color>][<width>]
//@                            [<lineCapEnd>])
//@     i.e.: (anchorLine (dx value)(dy value)(lineStyle value)(color value)
//@                       (width value) )
//@
//@ Example:
//@     (textbox (dx 50)(dy 5)
//@         (size (width 160)(height 100))
//@         (color #fffeb0)
//@         (border (width 5)(lineStyle dot)(color #000000))
//@         (text "This is a test of a textbox" (style "Textbox")
//@         (anchorLine (dx 0)(dy 0)(width 1)(color #ff0000)
//@                     (lineStyle dot)(lineCapEnd arrowhead))
//@     )
//@

class TextBoxLmdAnalyser : public LmdElementAnalyser
{
public:
    TextBoxLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTextBlockInfo box;

        // <location>
        if (get_mandatory(k_dx))
            box.set_position_x( get_location_child() );
        if (get_mandatory(k_dy))
            box.set_position_y( get_location_child() );

        // <size>
        if (get_mandatory(k_size))
            box.set_size( get_size_child() );

        // [<bgColor>])
        if (get_optional(k_color))
            box.set_bg_color( get_color_child() );

        // [<border>]
        if (get_optional(k_border))
            set_box_border(box);

        ImoTextBox* pTB = ImFactory::inject_text_box(pDoc, box);

        // <text>
        if (get_mandatory(k_text))
            set_box_text(pTB);

        // [<anchorLine>]
        if (get_optional(k_anchorLine))
            set_anchor_line(pTB);

        error_if_more_elements();

        add_to_model(pTB);
        return pTB;
    }

protected:

    void set_box_border(ImoTextBlockInfo& box)
    {
        ImoObj* pImo = analyse_child();
        if (pImo)
        {
            if (pImo->is_border_dto())
                box.set_border( static_cast<ImoBorderDto*>(pImo) );
            delete pImo;
        }
    }

    void set_box_text(ImoTextBox* pTB)
    {
        ImoObj* pImo = analyse_child();
        if (pImo)
        {
            if (pImo->is_score_text())
            {
                ImoScoreText* pText = static_cast<ImoScoreText*>(pImo);
                pTB->set_text(pText->get_text_info());
            }
            delete pImo;
        }
    }

    void set_anchor_line(ImoTextBox* pTB)
    {
        ImoObj* pImo = analyse_child();
        if (pImo)
        {
            if (pImo->is_line_style())
                pTB->set_anchor_line( static_cast<ImoLineStyle*>(pImo) );
            delete pImo;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <textItem> = (txt [<style>] string)
//@ <style> = (style <name>)
//@     if no style is specified it will inherit from parent style
//@
//$ <!ELEMENT txt %String;>
//$ <!ATTLIST txt %styleName;>
//$
//$ Example:
//$     <txt style='bold'>This is a text.</txt>
//$
class TextItemLmdAnalyser : public LmdElementAnalyser
{
public:
    TextItemLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
    {
    }

    ImoObj* do_analysis()
    {
        // [<style>]
        ImoStyle* pStyle = nullptr;
        if (has_attribute("style"))
            pStyle = get_doc_text_style( get_attribute("style") );

        // <string>
        string value = m_analysedNode.value();
        if (value.empty())
        {
            error_msg("txt: missing mandatory value 'string'. Element <txt> ignored.");
            return nullptr;
        }
        else
        {
            ImoTextItem* pText = create_text_item(value, get_node_id());
            pText->set_style(pStyle);

            add_to_model(pText);
            return pText;
        }
    }
};

//@--------------------------------------------------------------------------------------
//@ <textString> = (<textTag> string [<style>][<location>])
//@ <textTag> = { name | abbrev | text }
//@ <style> = (style <name>)
//@
//@ Compatibility 1.5:
//@ <style> is now mandatory
//@     For compatibility with 1.5, if no style is specified default style is
//@     assigned.
//@
class TextStringLmdAnalyser : public LmdElementAnalyser
{
protected:
    string m_styleName;

public:
    TextStringLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor, const string& styleName="Default style")
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_styleName(styleName)
    {
    }

    ImoObj* do_analysis()
    {
        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoScoreText* pText = static_cast<ImoScoreText*>(
                                        ImFactory::inject(k_imo_score_text, pDoc));
            pText->set_text(get_string_value());

            // [<style>]
            ImoStyle* pStyle = nullptr;
            if (get_optional(k_style))
                pStyle = get_text_style_child(m_styleName);
            else
            {
                ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
                if (pScore)     //in unit tests the score might not exist
                    pStyle = pScore->get_default_style();
            }
            pText->set_style(pStyle);

            // [<location>]
            while (more_children_to_analyse())
            {
                if (get_optional(k_dx))
                    pText->set_user_location_x( get_location_child() );
                else if (get_optional(k_dy))
                    pText->set_user_location_y( get_location_child() );
                else
                {
                    error_invalid_child();
                    move_to_next_child();
                }
            }
            error_if_more_elements();

            add_to_model(pText);
            return pText;
        }
        return nullptr;
    }

};


//@--------------------------------------------------------------------------------------
//@ <tie> = (tie num <tieType>[<bezier>][color] )   ;num = tie number. integer
//@ <tieType> = { start | stop }

class TieLmdAnalyser : public LmdElementAnalyser
{
public:
    TieLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTieDto* pInfo = static_cast<ImoTieDto*>(
                                ImFactory::inject(k_imo_tie_dto, pDoc));
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );

        // num
        if (get_mandatory(k_number))
            pInfo->set_tie_number( get_integer_value(0) );

        // <tieType> (label)
        if (!get_mandatory(k_label) || !set_tie_type(pInfo))
        {
            error_msg("Missing or invalid tie type. Tie ignored.");
            delete pInfo;
            return nullptr;
        }

        // [<bezier>]
        analyse_optional(k_bezier, pInfo);

        // [<color>]
        if (get_optional(k_color))
            pInfo->set_color( get_color_child() );

        return pInfo;
    }

protected:

    bool set_tie_type(ImoTieDto* pInfo)
    {
        const std::string& value = m_childToAnalyse.value();
        if (value == "start")
            pInfo->set_start(true);
        else if (value == "stop")
            pInfo->set_start(false);
        else
            return false;   //error
        return true;    //ok
    }
};

//@--------------------------------------------------------------------------------------
//@ <timeSignature> = (time <top><bottom>[<visible>][<location>])
//@ <top> = <num>
//@ <bottom> = <num>

class TimeSignatureLmdAnalyser : public LmdElementAnalyser
{
public:
    TimeSignatureLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>(
                                    ImFactory::inject(k_imo_time_signature, pDoc) );

        // <top> (num)
        if (get_mandatory(k_number))
            pTime->set_top_number( get_integer_value(2) );

        // <bottom> (num)
        if (get_mandatory(k_number))
            pTime->set_bottom_number( get_integer_value(4) );

        // [<visible>][<location>]
        analyse_staffobjs_options(pTime);

        add_to_model(pTime);
        return pTime;
    }

};

//@--------------------------------------------------------------------------------------
//@ <title> = (title <h-alignment> string [<style>][<location>])
//@ <h-alignment> = label: {left | center | right }
//@ <style> = (style name)
//@         name = string.  Must be a style name defined with defineStyle
//@
//@ Note:
//@     <h-alignment> overrides <style> (but doesn't modify it)
//@
//@ Examples:
//@     (title center "Prelude" (style "Title")
//@     (title center "Op. 28, No. 20" (style "Subtitle")
//@     (title right "F. Chopin" (style "Composer"(dy 30))

class TitleLmdAnalyser : public LmdElementAnalyser
{
public:
    TitleLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        // [<h-alignment>]
        int nAlignment = k_halign_left;
        if (get_mandatory(k_label))
            nAlignment = get_alignment_value(k_halign_center);

        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>(
                ImFactory::inject(k_imo_score_title, pDoc) );
            pTitle->set_text( get_string_value() );
            pTitle->set_h_align(nAlignment);

            // [<style>]
            if (get_optional(k_style))
                pTitle->set_style( get_text_style_child() );

            // [<location>]
            while (more_children_to_analyse())
            {
                if (get_optional(k_dx))
                    pTitle->set_user_location_x( get_location_child() );
                else if (get_optional(k_dy))
                    pTitle->set_user_location_y( get_location_child() );
                else
                {
                    error_invalid_child();
                    move_to_next_child();
                }
            }
            error_if_more_elements();

            add_to_model(pTitle);
            return pTitle;
        }
        return nullptr;
    }

};

//@--------------------------------------------------------------------------------------
//@ Old syntax: v1.5
//@ <tuplet> = (t { - | + <actualNotes>[<normalNotes>][<tupletOptions>] } )
//@ <actualNotes> = num
//@ <normalNotes> = num
//@ <tupletOptions> = noBracket
//@    future options: squaredBracket | curvedBracket |
//@                    numNone | numActual | numBoth
//@
//@ Abbreviations (old syntax. Deprecated since 1.6):
//@      (t -)     --> t-
//@      (t + n)   --> tn
//@      (t + n m) --> tn/m
//@
//@ New syntax: v1.6
//@ <tuplet> = (t <tupletID> { - | + <actualNotes>[<normalNotes>][<tupletOptions>] } )
//@ <tupletID> = integer number
//@ <actualNotes> = integer number
//@ <normalNotes> = integer number
//@ <tupletOptions> =  [<bracketType>] [<displayBracket>] [<displayNumber>]
//@ <bracketType> = (bracketType { squaredBracket | curvedBracket })
//@ <displayBracket> = (displayBracket { yes | no })
//@ <displayNumber> = (displayNumber { none | actual | both })

class TupletLmdAnalyser : public LmdElementAnalyser
{
public:
    TupletLmdAnalyser(LmdAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : LmdElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    ImoObj* do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
        set_default_values(pInfo);

        // [<tupletID>]     //optional for 1.5 compatibility. TO BE REMOVED
        if (get_optional(k_number))
            set_tuplet_id(pInfo);

        // { + | - }
        if (!get_mandatory(k_label) || !set_tuplet_type(pInfo))
        {
            error_msg("Missing or invalid tuplet type. Tuplet ignored.");
            delete pInfo;
            return nullptr;
        }

        if (pInfo->is_start_of_tuplet())
        {
            // <actualNotes>
            if (!get_mandatory(k_number) || !set_actual_notes(pInfo))
            {
                error_msg("Tuplet: missing or invalid actual notes number. Tuplet ignored.");
                delete pInfo;
                return nullptr;
            }

            // [<normalNotes>]
            if (get_optional(k_number))
                set_normal_notes(pInfo);
            if (pInfo->get_normal_number() == 0)
            {
                error_msg("Tuplet: Missing or invalid normal notes number. Tuplet ignored.");
                delete pInfo;
                return nullptr;
            }

            // [<tupletOptions>]
            analyse_tuplet_options(pInfo);
        }

        return pInfo;
    }

protected:

    void set_default_values(ImoTupletDto* pInfo)
    {
        pInfo->set_show_bracket( m_pAnalyser->get_current_show_tuplet_bracket() );
        pInfo->set_placement(k_placement_default);
        pInfo->set_line_number( m_pAnalyser->get_line_number(&m_analysedNode) );
    }

    void set_tuplet_id(ImoTupletDto* UNUSED(pInfo))
    {
        //TODO. For now tuplet id is not needed. Perhaps when implementing nested
        //      tuplets it will have any use.
        get_integer_value(0);
    }

    bool set_tuplet_type(ImoTupletDto* pInfo)
    {
        const std::string& value = m_childToAnalyse.value();
        if (value == "+")
            pInfo->set_tuplet_type(ImoTupletDto::k_start);
        else if (value == "-")
            pInfo->set_tuplet_type(ImoTupletDto::k_stop);
        else
            return false;   //error
        return true;    //ok
    }

    bool set_actual_notes(ImoTupletDto* pInfo)
    {
        int actual = get_integer_value(0);
        pInfo->set_actual_number(actual);
        if (actual == 2)
            pInfo->set_normal_number(3);   //duplet
        else if (actual == 3)
            pInfo->set_normal_number(2);   //triplet
        else if (actual == 4)
            pInfo->set_normal_number(6);
        else if (actual == 5)
            pInfo->set_normal_number(6);
        else
            pInfo->set_normal_number(0);  //required
        return true;    //ok
    }

    void set_normal_notes(ImoTupletDto* pInfo)
    {
        int normal = get_integer_value(0);
        pInfo->set_normal_number(normal);
    }

    void analyse_tuplet_options(ImoTupletDto* pInfo)
    {
        //@ <tupletOptions> =  [<bracketType>] [<displayBracket>] [<displayNumber>]
        //@ <bracketType> = (bracketType { squaredBracket | curvedBracket })
        //@ <displayBracket> = (displayBracket { yes | no })
        //@ <displayNumber> = (displayNumber { none | actual | both })

        int nShowBracket = m_pAnalyser->get_current_show_tuplet_bracket();
        int nShowNumber = m_pAnalyser->get_current_show_tuplet_number();
        while( more_children_to_analyse() )
        {
            m_childToAnalyse = get_child_to_analyse();
            ELdpElement type = get_type(&m_childToAnalyse);
            switch (type)
            {
                case k_label:
                {
                    const std::string& value = m_childToAnalyse.value();
                    if (value == "noBracket")
                        nShowBracket = k_yesno_no;
                    else
                        error_invalid_child();
                    break;
                }

                case k_displayBracket:
                {
                    m_childToAnalyse = m_childToAnalyse.first_child();
                    nShowBracket = get_yes_no_value(k_yesno_default);
                    break;
                }

                case k_displayNumber:
                {
                    m_childToAnalyse = m_childToAnalyse.first_child();
                    const std::string& value = m_childToAnalyse.value();
                    if (value == "none")
                        nShowNumber = ImoTuplet::k_number_none;
                    else if (value == "actual")
                        nShowNumber = ImoTuplet::k_number_actual;
                    else if (value == "both")
                        nShowNumber = ImoTuplet::k_number_both;
                    else
                    {
                        error_invalid_child();
                        nShowNumber = ImoTuplet::k_number_actual;
                    }
                    break;
                }

                default:
                    error_invalid_child();
            }

            move_to_next_child();
        }

        pInfo->set_show_bracket(nShowBracket);
        pInfo->set_show_number(nShowNumber);
        m_pAnalyser->set_current_show_tuplet_bracket(nShowBracket);
        m_pAnalyser->set_current_show_tuplet_number(nShowNumber);
    }

};



//=======================================================================================
// LmdElementAnalyser implementation
//=======================================================================================
ImoObj* LmdElementAnalyser::analyse_node(XmlNode* pNode)
{
    m_analysedNode = *pNode;
    move_to_first_child();
    return do_analysis();
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::error_missing_element(ELdpElement type)
{
    string parentName = m_analysedNode.name();
    const string& name = m_pLdpFactory->get_name(type);
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
               parentName + ": missing mandatory element '" + name + "'.");
    return false;
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::error_missing_element(const string& name)
{
    string parentName = m_analysedNode.name();
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
               parentName + ": missing mandatory element '" + name + "'.");
    return false;
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::report_msg(int numLine, const std::stringstream& msg)
{
    report_msg(numLine, msg.str());
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::report_msg(int numLine, const std::string& msg)
{
    m_reporter << "Line " << numLine << ". " << msg << endl;
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::has_attribute(const string& name)
{
    return has_attribute(&m_analysedNode, name);
}

//---------------------------------------------------------------------------------------
string LmdElementAnalyser::get_attribute(const string& name)
{
    return m_analysedNode.attribute_value(name);
}

//---------------------------------------------------------------------------------------
string LmdElementAnalyser::get_mandatory_string_attribute(const string& name,
                                  const string& sDefault, const string& element)
{
    string attrb = sDefault;
    if (has_attribute(&m_analysedNode, name))
        attrb = m_analysedNode.attribute_value(name);
    else if (sDefault.empty())
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            element + ": Missing mandatory attribute '" + name + "'." );
    else
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
            element + ": Missing mandatory attribute '" + name + "'. Value '"
            + sDefault + "' assumed.");

    return attrb;
}

//---------------------------------------------------------------------------------------
string LmdElementAnalyser::get_optional_string_attribute(const string& name,
                                                         const string& sDefault)
{
    if (has_attribute(&m_analysedNode, name))
        return m_analysedNode.attribute_value(name);
    else
        return sDefault;
}

//---------------------------------------------------------------------------------------
int LmdElementAnalyser::get_attribute_as_integer(const string& name, int nDefault)
{
    string number = m_analysedNode.attribute_value(name);
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
bool LmdElementAnalyser::get_mandatory(ELdpElement type)
{
    if (!more_children_to_analyse())
    {
        error_missing_element(type);
        return false;
    }

    m_childToAnalyse = get_child_to_analyse();
    if (get_type(&m_childToAnalyse) != type)
    {
        error_missing_element(type);
        return false;
    }

    move_to_next_child();
    return true;
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::get_mandatory(const string& type)
{
    if (!more_children_to_analyse())
    {
        error_missing_element(type);
        return false;
    }

    m_childToAnalyse = get_child_to_analyse();
    if (m_childToAnalyse.name() != type)
    {
        error_missing_element(type);
        return false;
    }

    move_to_next_child();
    return true;
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::analyse_mandatory(ELdpElement type, ImoObj* pAnchor)
{
    if (get_mandatory(type))
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::analyse_mandatory(const string& name, ImoObj* pAnchor)
{
    if (get_mandatory(name))
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::get_optional(ELdpElement type)
{
    if (more_children_to_analyse())
    {
        m_childToAnalyse = get_child_to_analyse();
        if (get_type(&m_childToAnalyse) == type)
        {
            move_to_next_child();
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::get_optional(const string& name)
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
bool LmdElementAnalyser::analyse_optional(ELdpElement type, ImoObj* pAnchor)
{
    if (get_optional(type))
    {
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::analyse_optional(const string& name, ImoObj* pAnchor)
{
    if (get_optional(name))
    {
        m_pAnalyser->analyse_node(&m_childToAnalyse, pAnchor);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::analyse_one_or_more(ELdpElement* pValid, int nValid)
{
    while(more_children_to_analyse())
    {
        m_childToAnalyse = get_child_to_analyse();

        ELdpElement type = get_type(&m_childToAnalyse);
        if (contains(type, pValid, nValid))
        {
            move_to_next_child();
            m_pAnalyser->analyse_node(&m_childToAnalyse);
        }
        else
        {
            string name = m_childToAnalyse.name();
            report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
                "Element '" + name + "' unknown or not possible here. Ignored.");
        }
        move_to_next_child();
    }
}

//---------------------------------------------------------------------------------------
bool LmdElementAnalyser::contains(ELdpElement type, ELdpElement* pValid, int nValid)
{
    for (int i=0; i < nValid; i++, pValid++)
        if (*pValid == type) return true;
    return false;
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::error_invalid_child()
{
    string name = m_childToAnalyse.name();
    if (name == "label")
        name += ":" + m_childToAnalyse.value();
    report_msg(m_pAnalyser->get_line_number(&m_childToAnalyse),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::error_msg(const string& msg)
{
    report_msg(m_pAnalyser->get_line_number(&m_analysedNode), msg);
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::error_if_more_elements()
{
    if (more_children_to_analyse())
    {
        string name = m_childToAnalyse.name();
        if (name == "label")
            name += ":" + m_childToAnalyse.value();
        report_msg(m_pAnalyser->get_line_number(&m_analysedNode),
                "Element '" + m_analysedNode.name()
                + "': too many parameters. Extra parameters from '"
                + name + "' have been ignored.");
    }
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::analyse_staffobjs_options(ImoStaffObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <staffobjOptions> = { <staffNum> | <componentOptions> }
    //@ <numStaff> = pn

    // [<numStaff>]
    if (more_children_to_analyse())
    {
        m_childToAnalyse = get_child_to_analyse();
        if (m_childToAnalyse.name() == "staff")
        {
            get_num_staff();
            move_to_next_child();
        }
    }

    //set staff: either found value or inherited one
    pSO->set_staff( m_pAnalyser->get_current_staff() );

    analyse_scoreobj_options(pSO);
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::analyse_scoreobj_options(ImoScoreObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <componentOptions> = { <visible> | <location> | <color> }
    //@ <visible> = (visible {yes | no})
    //@ <location> = { (dx num) | (dy num) }    ;num in Tenths
    //@ <color> = value                         ;value in #rrggbb hex format

    // [ { <visible> | <location> | <color> }* ]
    while( more_children_to_analyse() )
    {
        m_childToAnalyse = get_child_to_analyse();
        ELdpElement type = get_type(&m_childToAnalyse);
        switch (type)
        {
            case k_visible:
            {
                m_childToAnalyse = m_childToAnalyse.first_child();
                pSO->set_visible( get_bool_value(true) );
                break;
            }
            case k_color:
            {
                pSO->set_color( get_color_child() );
                break;
            }
            case k_dx:
            {
                pSO->set_user_location_x( get_location_child() );
                break;
            }
            case k_dy:
            {
                pSO->set_user_location_y( get_location_child() );
                break;
            }
            default:
                return;
        }

        move_to_next_child();
    }
}

//---------------------------------------------------------------------------------------
void LmdElementAnalyser::add_to_model(ImoObj* pImo)
{
    int ldpNodeType = get_type(&m_analysedNode);
    //pImo->set_id(get_id(&m_analysedNode));        //transfer id
    Linker linker( m_pAnalyser->get_document_being_analysed() );
    linker.add_child_to_model(m_pAnchor, pImo, ldpNodeType);
}




//=======================================================================================
// LmdAnalyser implementation
//=======================================================================================
LmdAnalyser::LmdAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc,
                         XmlParser* parser)
    : Analyser()
    , m_reporter(reporter)
    , m_libraryScope(libraryScope)
    , m_pDoc(pDoc)
    , m_pParser(parser)
    , m_pLdpFactory(libraryScope.ldp_factory())
    , m_pTiesBuilder(nullptr)
    , m_pOldTiesBuilder(nullptr)
    , m_pBeamsBuilder(nullptr)
    , m_pOldBeamsBuilder(nullptr)
    , m_pTupletsBuilder(nullptr)
    , m_pSlursBuilder(nullptr)
    , m_pCurScore(nullptr)
    , m_pLastScore(nullptr)
    , m_pImoDoc(nullptr)
    , m_scoreVersion(0)
    , m_fInstrIdRequired(false)
    , m_pTree()
    , m_fileLocator("")
    , m_nShowTupletBracket(k_yesno_default)
    , m_nShowTupletNumber(k_yesno_default)
    , m_pLastNote(nullptr)
{
    m_NameToTag["clef"] = k_tag_clef;
    m_NameToTag["content"] = k_tag_content;
    m_NameToTag["color"] = k_tag_color;
    m_NameToTag["defineStyle"] = k_tag_defineStyle;
    m_NameToTag["dynamic"] = k_tag_dynamic;
    m_NameToTag["group"] = k_tag_group;
    m_NameToTag["image"] = k_tag_image;
    m_NameToTag["instrument"] = k_tag_instrument;
    m_NameToTag["itemizedlist"] = k_tag_itemizedlist;
    m_NameToTag["ldpmusic"] = k_tag_ldpmusic;
    m_NameToTag["lenmusdoc"] = k_tag_lenmusdoc;
    m_NameToTag["link"] = k_tag_link;
    m_NameToTag["listitem"] = k_tag_listitem;
    m_NameToTag["musicData"] = k_tag_musicData;
    m_NameToTag["orderedlist"] = k_tag_orderedlist;
    m_NameToTag["para"] = k_tag_para;
    m_NameToTag["param"] = k_tag_param;
    m_NameToTag["parts"] = k_tag_parts;
    m_NameToTag["score"] = k_tag_score;
    m_NameToTag["scorePlayer"] = k_tag_scorePlayer;
    m_NameToTag["section"] = k_tag_section;
    m_NameToTag["styles"] = k_tag_styles;
    m_NameToTag["table"] = k_tag_table;
    m_NameToTag["tableCell"] = k_tag_tableCell;
    m_NameToTag["tableColumn"] = k_tag_tableColumn;
    m_NameToTag["tableBody"] = k_tag_tableBody;
    m_NameToTag["tableHead"] = k_tag_tableHead;
    m_NameToTag["tableRow"] = k_tag_tableRow;
    m_NameToTag["txt"] = k_tag_txt;
}

//---------------------------------------------------------------------------------------
LmdAnalyser::~LmdAnalyser()
{
    delete_relation_builders();
    m_NameToTag.clear();
}

//---------------------------------------------------------------------------------------
void LmdAnalyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pOldTiesBuilder;
    delete m_pBeamsBuilder;
    delete m_pOldBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* LmdAnalyser::analyse_tree_and_get_object(XmlNode* root)
{
    //TODO_X
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW LmdTiesBuilder(m_reporter, this);
    m_pOldTiesBuilder = LOMSE_NEW OldLmdTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW LmdBeamsBuilder(m_reporter, this);
    m_pOldBeamsBuilder = LOMSE_NEW OldLmdBeamsBuilder(m_reporter, this);
    m_pTupletsBuilder = LOMSE_NEW LmdTupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW LmdSlursBuilder(m_reporter, this);

    m_pTree = root;
    m_curStaff = 0;
    m_curVoice = 1;
    return analyse_node(root);
}

//---------------------------------------------------------------------------------------
ImoObj* LmdAnalyser::analyse_tree(XmlNode* tree, const string& locator)
{
    m_fileLocator = locator;
    return analyse_tree_and_get_object(tree);
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::get_line_number(XmlNode* node)
{
    return m_pParser->get_line_number(node);
}

//---------------------------------------------------------------------------------------
ImoObj* LmdAnalyser::analyse_node(XmlNode* pNode, ImoObj* pAnchor)
{
    LmdElementAnalyser* a = new_analyser( pNode->name(), pAnchor );
    ImoObj* pImo = a->analyse_node(pNode);
    delete a;
    return pImo;
}

//---------------------------------------------------------------------------------------
void LmdAnalyser::add_relation_info(ImoObj* pDto)
{
    // factory method to deal withh all relations

    if (pDto->is_tie_dto())
        m_pTiesBuilder->add_item_info(static_cast<ImoTieDto*>(pDto));
    else if (pDto->is_slur_dto())
        m_pSlursBuilder->add_item_info(static_cast<ImoSlurDto*>(pDto));
    else if (pDto->is_beam_dto())
        m_pBeamsBuilder->add_item_info(static_cast<ImoBeamDto*>(pDto));
    else if (pDto->is_tuplet_dto())
        m_pTupletsBuilder->add_item_info(static_cast<ImoTupletDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void LmdAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
    m_pOldBeamsBuilder->clear_pending_old_beams();
    m_pTupletsBuilder->clear_pending_items();
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::set_score_version(const string& version)
{
    //version is a string "major.minor". Extract major and minor and compose
    //and integer 100*major+minor

    m_scoreVersion = 0;
    size_t i = version.find('.');
    if (i != string::npos)
    {
        string major = version.substr(0, i);
        if ( to_integer(major, &m_scoreVersion) )
            return m_scoreVersion;

        m_scoreVersion *= 100;
        string minor = version.substr(i+1);
        int nMinor;
        to_integer(minor, &nMinor);

        m_scoreVersion += nMinor;
    }
    m_pCurScore->set_version(m_scoreVersion);

    return m_scoreVersion;
}

//---------------------------------------------------------------------------------------
bool LmdAnalyser::to_integer(const string& text, int* pResult)
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
int LmdAnalyser::ldp_name_to_key_type(const string& value)
{
    if (value == "C")
        return k_key_C;
    else if (value == "G")
        return k_key_G;
    else if (value == "D")
        return k_key_D;
    else if (value == "A")
        return k_key_A;
    else if (value == "E")
        return k_key_E;
    else if (value == "B")
        return k_key_B;
    else if (value == "F+")
        return k_key_Fs;
    else if (value == "C+")
        return k_key_Cs;
    else if (value == "C-")
        return k_key_Cf;
    else if (value == "G-")
        return k_key_Gf;
    else if (value == "D-")
        return k_key_Df;
    else if (value == "A-")
        return k_key_Af;
    else if (value == "E-")
        return k_key_Ef;
    else if (value == "B-")
        return k_key_Bf;
    else if (value == "F")
        return k_key_F;
    else if (value == "a")
        return k_key_a;
    else if (value == "e")
        return k_key_e;
    else if (value == "b")
        return k_key_b;
    else if (value == "f+")
        return k_key_fs;
    else if (value == "c+")
        return k_key_cs;
    else if (value == "g+")
        return k_key_gs;
    else if (value == "d+")
        return k_key_ds;
    else if (value == "a+")
        return k_key_as;
    else if (value == "a-")
        return k_key_af;
    else if (value == "e-")
        return k_key_ef;
    else if (value == "b-")
        return k_key_bf;
    else if (value == "f")
        return k_key_f;
    else if (value == "c")
        return k_key_c;
    else if (value == "g")
        return k_key_g;
    else if (value == "d")
        return k_key_d;
    else
        return k_key_undefined;
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::ldp_name_to_clef_type(const string& value)
{
    if (value == "G")
        return k_clef_G2;
    else if (value == "F4")
        return k_clef_F4;
    else if (value == "F3")
        return k_clef_F3;
    else if (value == "C1")
        return k_clef_C1;
    else if (value == "C2")
        return k_clef_C2;
    else if (value == "C3")
        return k_clef_C3;
    else if (value == "C4")
        return k_clef_C4;
    else if (value == "percussion")
        return k_clef_percussion;
    else if (value == "C3")
        return k_clef_C3;
    else if (value == "C5")
        return k_clef_C5;
    else if (value == "F5")
        return k_clef_F5;
    else if (value == "G1")
        return k_clef_G1;
    else if (value == "8_G")
        return k_clef_8_G2;
    else if (value == "G_8")
        return k_clef_G2_8;
    else if (value == "8_F4")
        return k_clef_8_F4;
    else if (value == "F4_8")
        return k_clef_F4_8;
    else if (value == "15_G")
        return k_clef_15_G2;
    else if (value == "G_15")
        return k_clef_G2_15;
    else if (value == "15_F4")
        return k_clef_15_F4;
    else if (value == "F4_15")
        return k_clef_F4_15;
#if LOMSE_COMPATIBILITY_LDP_1_5
    else if (value == "F" || value == "bass")
        return k_clef_F4;
    else if (value == "trebble")
        return k_clef_G2;
#endif
    else
        return k_clef_undefined;
}

//---------------------------------------------------------------------------------------
bool LmdAnalyser::ldp_pitch_to_components(const string& pitch, int *step, int* octave,
                                       EAccidentals* accidentals)
{
    // Analyzes string pitch (LDP format), extracts its parts (step, octave and
    // accidentals) and stores them in the corresponding parameters.
    // Returns true if error (pitch is not a valid pitch name)
    //
    // In LDP pitch is represented as a combination of the step of the diatonic
    // scale, the chromatic alteration, and the octave.
    //    - The accidentals parameter represents chromatic alteration (does not
    //      include key alterations)
    //    - The octave element is represented by the numbers 0 to 9, where 4
    //      is the octave started by middle C.
    //
    // pitch must be trimed (no spaces before or after real data) and lower case

    size_t i = pitch.length() - 1;
    if (i < 1)
        return true;   //error

    *octave = to_octave(pitch[i--]);
    if (*octave == -1)
        return true;   //error

    *step = to_step(pitch[i--]);
    if (*step == -1)
        return true;   //error

    if (++i == 0)
    {
        *accidentals = k_no_accidentals;
        return false;   //no error
    }
    else
        *accidentals = to_accidentals(pitch.substr(0, i));
    if (*accidentals == k_invalid_accidentals)
        return true;   //error

    return false;  //no error
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::to_step(const char& letter)
{
	switch (letter)
    {
		case 'a':	return k_step_A;
		case 'b':	return k_step_B;
		case 'c':	return k_step_C;
		case 'd':	return k_step_D;
		case 'e':	return k_step_E;
		case 'f':	return k_step_F;
		case 'g':	return k_step_G;
	}
	return -1;
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::to_octave(const char& letter)
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
EAccidentals LmdAnalyser::to_accidentals(const std::string& accidentals)
{
    switch (accidentals.length())
    {
        case 0:
            return k_no_accidentals;
            break;

        case 1:
            if (accidentals[0] == '+')
                return k_sharp;
            else if (accidentals[0] == '-')
                return k_flat;
            else if (accidentals[0] == '=')
                return k_natural;
            else if (accidentals[0] == 'x')
                return k_double_sharp;
            else
                return k_invalid_accidentals;
            break;

        case 2:
            if (accidentals.compare(0, 2, "++") == 0)
                return k_sharp_sharp;
            else if (accidentals.compare(0, 2, "--") == 0)
                return k_flat_flat;
            else if (accidentals.compare(0, 2, "=-") == 0)
                return k_natural_flat;
            else
                return k_invalid_accidentals;
            break;

        default:
            return k_invalid_accidentals;
    }
}

//---------------------------------------------------------------------------------------
LmdElementAnalyser* LmdAnalyser::new_analyser(const string& name, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch ( name_to_tag(name) )
    {
//        case k_tag_abbrev:          return LOMSE_NEW InstrNameLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_anchorLine:      return LOMSE_NEW AnchorLineLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_barline:         return LOMSE_NEW BarlineLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_beam:            return LOMSE_NEW BeamLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_bezier:          return LOMSE_NEW BezierLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_border:          return LOMSE_NEW BorderLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_chord:           return LOMSE_NEW ChordLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_clef:            return LOMSE_NEW ClefLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_content:         return LOMSE_NEW ContentLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
////        case k_tag_creationMode:    return LOMSE_NEW ContentLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_color:           return LOMSE_NEW ColorLmdAnalyser(this, m_reporter, m_libraryScope);
//        case k_tag_cursor:          return LOMSE_NEW CursorLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_defineStyle:     return LOMSE_NEW DefineStyleLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_endPoint:        return LOMSE_NEW PointLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_dynamic:         return LOMSE_NEW DynamicLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_fermata:         return LOMSE_NEW FermataLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_figuredBass:     return LOMSE_NEW FiguredBassLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_font:            return LOMSE_NEW FontLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_goBack:          return LOMSE_NEW GoBackFwdLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_goFwd:           return LOMSE_NEW GoBackFwdLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_group:           return LOMSE_NEW GroupLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_image:           return LOMSE_NEW ImageLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_itemizedlist:    return LOMSE_NEW ListLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_infoMIDI:        return LOMSE_NEW InfoMidiLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_instrument:      return LOMSE_NEW InstrumentLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_key_signature:   return LOMSE_NEW KeySignatureLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_ldpmusic:        return LOMSE_NEW LdpmusicLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_lenmusdoc:       return LOMSE_NEW LenmusdocLmdAnalyser(this, m_reporter, m_libraryScope);
//        case k_tag_line:            return LOMSE_NEW LineLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_link:            return LOMSE_NEW LinkLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_listitem:        return LOMSE_NEW ListItemLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_metronome:       return LOMSE_NEW MetronomeLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_musicData:       return LOMSE_NEW MusicDataLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_na:              return LOMSE_NEW NoteRestLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_newSystem:       return LOMSE_NEW ControlLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_note:            return LOMSE_NEW NoteRestLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_opt:             return LOMSE_NEW OptLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_orderedlist:     return LOMSE_NEW ListLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_pageLayout:      return LOMSE_NEW PageLayoutLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_pageMargins:     return LOMSE_NEW PageMarginsLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_pageSize:        return LOMSE_NEW PageSizeLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_para:            return LOMSE_NEW ParagraphLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_param:           return LOMSE_NEW ParamLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_parts:           return LOMSE_NEW PartsLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_rest:            return LOMSE_NEW NoteRestLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_settings:        return LOMSE_NEW SettingsLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_score:           return LOMSE_NEW ScoreLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_scorePlayer:     return LOMSE_NEW ScorePlayerLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_section:         return LOMSE_NEW SectionLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_size:            return LOMSE_NEW SizeLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_slur:            return LOMSE_NEW SlurLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_spacer:          return LOMSE_NEW SpacerLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_staff:           return LOMSE_NEW StaffLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
////        case k_tag_symbol:          return LOMSE_NEW XxxxxxxLmdAnalyser(this, m_reporter, m_libraryScope);
////        case k_tag_symbolSize:      return LOMSE_NEW XxxxxxxLmdAnalyser(this, m_reporter, m_libraryScope);
//        case k_tag_startPoint:      return LOMSE_NEW PointLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_styles:          return LOMSE_NEW StylesLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_systemLayout:    return LOMSE_NEW SystemLayoutLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_systemMargins:   return LOMSE_NEW SystemMarginsLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_table:           return LOMSE_NEW TableLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_tableCell:       return LOMSE_NEW TableCellLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_tableColumn:     return LOMSE_NEW TableColumnLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_tableBody:       return LOMSE_NEW TableBodyLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_tableHead:       return LOMSE_NEW TableHeadLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_tableRow:        return LOMSE_NEW TableRowLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tag_txt:             return LOMSE_NEW TextItemLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_text:            return LOMSE_NEW TextStringLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_textbox:         return LOMSE_NEW TextBoxLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_time_signature:  return LOMSE_NEW TimeSignatureLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_tie:             return LOMSE_NEW TieLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_title:           return LOMSE_NEW TitleLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_tag_tuplet:          return LOMSE_NEW TupletLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
////        case k_tag_undoData:        return LOMSE_NEW XxxxxxxLmdAnalyser(this, m_reporter, m_libraryScope, pAnchor);

        default:
            return LOMSE_NEW NullLmdAnalyser(this, m_reporter, m_libraryScope, name);
    }
}

//---------------------------------------------------------------------------------------
int LmdAnalyser::name_to_tag(const string& name) const
{
	map<string, int>::const_iterator it = m_NameToTag.find(name);
	if (it != m_NameToTag.end())
		return it->second;
    else
        return k_tag_undefined;
}



//=======================================================================================
// LmdTiesBuilder implementation
//=======================================================================================
void LmdTiesBuilder::add_relation_to_staffobjs(ImoTieDto* pEndDto)
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
bool LmdTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void LmdTiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
{
    ImoNote* pStartNote = pStartDto->get_note();
    ImoNote* pEndNote = pEndDto->get_note();
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, pDoc));
    pTie->set_tie_number( pStartDto->get_tie_number() );
    pTie->set_color( pStartDto->get_color() );

    ImoTieData* pStartData = ImFactory::inject_tie_data(pDoc, pStartDto);
    pStartNote->include_in_relation(pDoc, pTie, pStartData);

    ImoTieData* pEndData = ImFactory::inject_tie_data(pDoc, pEndDto);
    pEndNote->include_in_relation(pDoc, pTie, pEndData);

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);
}

//---------------------------------------------------------------------------------------
void LmdTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}


//=======================================================================================
// OldLmdTiesBuilder implementation
//=======================================================================================
OldLmdTiesBuilder::OldLmdTiesBuilder(ostream& reporter, LmdAnalyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pStartNoteTieOld(nullptr)
{
}

//---------------------------------------------------------------------------------------
bool OldLmdTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void OldLmdTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldLmdTiesBuilder::error_invalid_tie_old_syntax(int line)
{
    m_reporter << "Line " << line
               << ". No note found to match old syntax tie. Tie ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldLmdTiesBuilder::start_old_tie(ImoNote* pNote, XmlNode* pOldTie)
{
    if (m_pStartNoteTieOld)
        create_tie_if_old_syntax_tie_pending(pNote);

    m_pStartNoteTieOld = pNote;
    m_pOldTieParam = pOldTie;
}

//---------------------------------------------------------------------------------------
void OldLmdTiesBuilder::create_tie_if_old_syntax_tie_pending(ImoNote* pEndNote)
{
    if (!m_pStartNoteTieOld)
        return;

    if (notes_can_be_tied(m_pStartNoteTieOld, pEndNote))
    {
        tie_notes(m_pStartNoteTieOld, pEndNote);
        m_pStartNoteTieOld = nullptr;
    }
    else if (m_pStartNoteTieOld->get_voice() == pEndNote->get_voice())
    {
        error_invalid_tie_old_syntax( 0 );  //TODO_X    get_line_number(m_pOldTieParam) );
        m_pStartNoteTieOld = nullptr;
    }
    else
    {
        //  wait to see if it is possible to tie with next note
    }
}

//---------------------------------------------------------------------------------------
void OldLmdTiesBuilder::tie_notes(ImoNote* pStartNote, ImoNote* pEndNote)
{
    Document* pDoc = m_pAnalyser->get_document_being_analysed();
    ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, pDoc));

    ImoTieDto startDto;
    startDto.set_start(true);
    ImoTieData* pStartData = ImFactory::inject_tie_data(pDoc, &startDto);
    pStartNote->include_in_relation(pDoc, pTie, pStartData);

    ImoTieDto endDto;
    endDto.set_start(false);
    ImoTieData* pEndData = ImFactory::inject_tie_data(pDoc, &endDto);
    pEndNote->include_in_relation(pDoc, pTie, pEndData);

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);
}



//=======================================================================================
// LmdSlursBuilder implementation
//=======================================================================================
void LmdSlursBuilder::add_relation_to_staffobjs(ImoSlurDto* pEndInfo)
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
// LmdBeamsBuilder implementation
//=======================================================================================
void LmdBeamsBuilder::add_relation_to_staffobjs(ImoBeamDto* pEndInfo)
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

    //AWARE: LDP v1.6 requires full item description, Autobeamer is not needed
    //AutoBeamer autobeamer(pBeam);
    //autobeamer.do_autobeam();
}


//=======================================================================================
// OldLmdBeamsBuilder implementation
//=======================================================================================
OldLmdBeamsBuilder::OldLmdBeamsBuilder(ostream& reporter, LmdAnalyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
{
}

//---------------------------------------------------------------------------------------
OldLmdBeamsBuilder::~OldLmdBeamsBuilder()
{
    clear_pending_old_beams();
}

//---------------------------------------------------------------------------------------
void OldLmdBeamsBuilder::add_old_beam(ImoBeamDto* pInfo)
{
    m_pendingOldBeams.push_back(pInfo);
}

//---------------------------------------------------------------------------------------
void OldLmdBeamsBuilder::clear_pending_old_beams()
{
    std::list<ImoBeamDto*>::iterator it;
    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
    {
        error_no_end_old_beam(*it);
        delete *it;
    }
    m_pendingOldBeams.clear();
}

//---------------------------------------------------------------------------------------
bool OldLmdBeamsBuilder::is_old_beam_open()
{
    return m_pendingOldBeams.size() > 0;
}

//---------------------------------------------------------------------------------------
void OldLmdBeamsBuilder::error_no_end_old_beam(ImoBeamDto* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No matching 'g-' element for 'g+'. Beam ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldLmdBeamsBuilder::close_old_beam(ImoBeamDto* pInfo)
{
    add_old_beam(pInfo);
    do_create_old_beam();
}

//---------------------------------------------------------------------------------------
void OldLmdBeamsBuilder::do_create_old_beam()
{
    Document* pDoc = m_pAnalyser->get_document_being_analysed();
    ImoBeam* pBeam = static_cast<ImoBeam*>(ImFactory::inject(k_imo_beam, pDoc));
    std::list<ImoBeamDto*>::iterator it;
    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoBeamData* pData = ImFactory::inject_beam_data(pDoc, *it);
        pNR->include_in_relation(pDoc, pBeam, pData);
        delete *it;
    }
    m_pendingOldBeams.clear();

    AutoBeamer autobeamer(pBeam);
    autobeamer.do_autobeam();
}



//=======================================================================================
// LmdTupletsBuilder implementation
//=======================================================================================
void LmdTupletsBuilder::add_relation_to_staffobjs(ImoTupletDto* pEndDto)
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


}   //namespace lomse
