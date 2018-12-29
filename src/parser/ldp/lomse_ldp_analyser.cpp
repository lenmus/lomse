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

#include "lomse_ldp_analyser.h"

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
#include "lomse_ldp_parser.h"
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
#include "lomse_im_algorithms.h"
#include "lomse_autobeamer.h"

using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// The syntax analyser is based on the Interpreter pattern ()
//
// The basic idea is to have a class for each language symbol, terminal or nonterminal
// (classes derived from ElementAnalyser). The parse tree created by the Parser is an
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

class ElementAnalyser
{
protected:
    ostream& m_reporter;
    LdpAnalyser* m_pAnalyser;
    LibraryScope& m_libraryScope;
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    ElementAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor=nullptr)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor)
        //
        , m_pAnalysedNode(nullptr)
        , m_pParamToAnalyse(nullptr)
        , m_pNextParam(nullptr)
        , m_pNextNextParam(nullptr)
    {
    }
    virtual ~ElementAnalyser() {}
    void analyse_node(LdpElement* pNode);

protected:

    //analysis
    virtual void do_analysis() = 0;

    //error reporting
    bool error_missing_element(ELdpElement type);
    void report_msg(int numLine, const std::string& msg);
    void report_msg(int numLine, const std::stringstream& msg);
    void error_if_more_elements();
    void error_invalid_param();
    void error_msg(const string& msg);

    //helpers, to simplify writting grammar rules
    LdpElement* m_pAnalysedNode;
    LdpElement* m_pParamToAnalyse;
    LdpElement* m_pNextParam;
    LdpElement* m_pNextNextParam;

    bool get_mandatory(ELdpElement type);
    void analyse_mandatory(ELdpElement type, ImoObj* pAnchor=nullptr);
    bool get_optional(ELdpElement type);
    bool analyse_optional(ELdpElement type, ImoObj* pAnchor=nullptr);
    void analyse_one_or_more(ELdpElement* pValid, int nValid);
    void analyse_staffobjs_options(ImoStaffObj* pSO);
    void analyse_scoreobj_options(ImoScoreObj* pSO);
    inline ImoObj* proceed(ELdpElement UNUSED(type), ImoObj* pAnchor)
    {
        return m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
    }

    //building the model
    void add_to_model(ImoObj* pImo);
    void create_measure_info_if_necessary();

    //auxiliary
    inline ImoId get_node_id() { return m_pAnalysedNode->get_id(); }
    bool contains(ELdpElement type, ELdpElement* pValid, int nValid);
    inline const string& get_document_locator() {
        return m_pAnalyser->get_document_locator();
    }

    //-----------------------------------------------------------------------------------
    inline void post_event(SpEventInfo event)
    {
        m_libraryScope.post_event(event);
    }

    //-----------------------------------------------------------------------------------
    inline bool more_params_to_analyse() {
        return m_pNextParam != nullptr;
    }

    //-----------------------------------------------------------------------------------
    inline LdpElement* get_param_to_analyse() {
        return m_pNextParam;
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_next_param() {
        m_pNextParam = m_pNextNextParam;
        prepare_next_one();
    }

    //-----------------------------------------------------------------------------------
    inline void prepare_next_one() {
        if (m_pNextParam)
            m_pNextNextParam = m_pNextParam->get_next_sibling();
        else
            m_pNextNextParam = nullptr;
    }

    //-----------------------------------------------------------------------------------
    inline void move_to_first_param() {
        m_pNextParam = m_pAnalysedNode->get_first_child();
        prepare_next_one();
    }

    //-----------------------------------------------------------------------------------
    void get_num_staff()
    {
        string staff = m_pParamToAnalyse->get_value().substr(1);
        int nStaff;
        //http://www.codeguru.com/forum/showthread.php?t=231054
        std::istringstream iss(staff);
        if ((iss >> std::dec >> nStaff).fail())
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid staff 'p" + staff + "'. Replaced by 'p1'.");
            m_pAnalyser->set_current_staff(0);
        }
        else
            m_pAnalyser->set_current_staff(--nStaff);
    }

    //-----------------------------------------------------------------------------------
    void get_voice()
    {
        string voice = m_pParamToAnalyse->get_value().substr(1);
        int nVoice;
        std::istringstream iss(voice);
        if ((iss >> std::dec >> nVoice).fail())
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid voice 'v" + voice + "'. Replaced by 'v1'.");
            m_pAnalyser->set_current_voice(1);
        }
        else
            m_pAnalyser->set_current_voice(nVoice);
    }

    //-----------------------------------------------------------------------------------
    bool is_long_value()
    {
        string number = m_pParamToAnalyse->get_value();
        long nNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> nNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    long get_long_value(long nDefault=0L)
    {
        string number = m_pParamToAnalyse->get_value();
        long nNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> nNumber).fail())
        {
            stringstream replacement;
            replacement << nDefault;
            report_msg(m_pParamToAnalyse->get_line_number(),
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
        string number = m_pParamToAnalyse->get_value();
        float rNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> rNumber).fail());
    }

    //-----------------------------------------------------------------------------------
    float get_float_value(float rDefault=0.0f)
    {
        string number = m_pParamToAnalyse->get_value();
        float rNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> rNumber).fail())
        {
            stringstream replacement;
            replacement << rDefault;
            report_msg(m_pParamToAnalyse->get_line_number(),
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
        string value = m_pParamToAnalyse->get_value();
        return  value == "true" || value == "yes"
             || value == "false" || value == "no" ;
    }

    //-----------------------------------------------------------------------------------
    bool get_bool_value(bool fDefault=false)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "true" || value == "yes")
            return true;
        else if (value == "false" || value == "no")
            return false;
        else
        {
            stringstream replacement;
            replacement << fDefault;
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid boolean value '" + value + "'. Replaced by '"
                + replacement.str() + "'.");
            return fDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    int get_yes_no_value(int nDefault)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "yes")
            return k_yesno_yes;
        else if (value == "no")
            return k_yesno_no;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid yes/no value '" + value + "'. Replaced by default.");
            return nDefault;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_string_value()
    {
        return m_pParamToAnalyse->get_value();
    }

    //-----------------------------------------------------------------------------------
    EHAlign get_alignment_value(EHAlign defaultValue)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
        if (value == "left")
            return k_halign_left;
        else if (value == "right")
            return k_halign_right;
        else if (value == "center")
            return k_halign_center;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Invalid alignment value '" + value + "'. Assumed 'center'.");
            return defaultValue;
        }
    }

    //-----------------------------------------------------------------------------------
    string get_string_param()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return m_pParamToAnalyse->get_value();
    }

    //-----------------------------------------------------------------------------------
    Color get_color_param()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr);
        Color color;
        if (pImo->is_color_dto())
        {
            ImoColorDto* pColor = static_cast<ImoColorDto*>( pImo );
            color = pColor->get_color();
        }
        delete pImo;
        return color;
    }

    float get_font_size_value()
    {
        const string& value = m_pParamToAnalyse->get_value();
        int size = static_cast<int>(value.size()) - 2;
        string points = value.substr(0, size);
        float rNumber;
        std::istringstream iss(points);
        if ((iss >> std::dec >> rNumber).fail())
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid size '" + value + "'. Replaced by '12'.");
            return 12.0f;
        }
        else
            return rNumber;
    }

    float get_font_size_param()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_font_size_value();
    }

    //-----------------------------------------------------------------------------------
    ImoStyle* get_text_style_param(const string& defaulName="Default style")
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
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
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pScore->get_style_or_default(defaulName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    TPoint get_point_param()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr);
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
    TSize get_size_param()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr);
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
    float get_location_param()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(0.0f);
    }

    //-----------------------------------------------------------------------------------
    float get_width_param(float rDefault=1.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_height_param(float rDefault=1.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_float_param(float rDefault=1.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_lenght_param(float rDefault=0.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
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
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pDoc->get_style_or_default(styleName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    ELineStyle get_line_style_param()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        const std::string& value = m_pParamToAnalyse->get_value();
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
            report_msg(m_pAnalysedNode->get_line_number(),
                "Element 'lineStyle': Invalid value '" + value
                + "'. Replaced by 'solid'." );
            return k_line_solid;
        }
    }

    //-----------------------------------------------------------------------------------
    ELineCap get_line_cap_param()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        const std::string& value = m_pParamToAnalyse->get_value();
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
            report_msg(m_pAnalysedNode->get_line_number(),
                "Element 'lineCap': Invalid value '" + value
                + "'. Replaced by 'none'." );
            return k_cap_none;
        }
    }

    //-----------------------------------------------------------------------------------
    int get_placement(int defValue)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "above")
            return k_placement_above;
        else if (value == "below")
            return k_placement_below;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown value '" + value + "' for <placement>. Replaced by '"
                + (defValue == k_placement_above ? "above'." : "below'.") );
            return defValue;
        }
    }

    //-----------------------------------------------------------------------------------
    void check_visible(ImoInlinesContainer* pCO)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "visible")
            pCO->set_visible(true);
        else if (value == "noVisible")
            pCO->set_visible(false);
        else
        {
            error_invalid_param();
            pCO->set_visible(true);
        }
    }

    //-----------------------------------------------------------------------------------
    NoteTypeAndDots get_note_type_and_dots()
    {
        string duration = m_pParamToAnalyse->get_value();
        NoteTypeAndDots figdots = ldp_duration_to_components(duration);
        if (figdots.noteType == k_unknown_notetype)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown note/rest duration '" + duration + "'. Replaced by 'q'.");
            figdots.noteType = k_quarter;
        }
        return figdots;
    }

    //-----------------------------------------------------------------------------------
    void analyse_noterest_attachments(ImoStaffObj* pAnchor)
    {
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (is_noterest_attachment(type))
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
            else
                error_invalid_param();

            move_to_next_param();
        }
    }

    //-----------------------------------------------------------------------------------
    void analyse_staffobj_attachments(ImoStaffObj* pAnchor)
    {
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (is_staffobj_attachment(type))
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
            else
                error_invalid_param();

            move_to_next_param();
        }
    }

    //-----------------------------------------------------------------------------------
    //@ attachment : { `text` | `textbox` | `line` | `fermata` | `dynamics` |
    //@            :   `metronome` | `accent` | `articulation` | `caesura` |
    //@            :   `breathMark` | `technical` | `ornament` }
    //@
    bool is_noterest_attachment(int type)
    {
        return     type == k_text
                || type == k_textbox
                || type == k_line
                || type == k_fermata
                || type == k_dynamics_mark
                //accents
                || type == k_accent
                || type == k_legato_duro
                || type == k_marccato
                || type == k_marccato_legato
                || type == k_marccato_staccato
                || type == k_marccato_staccatissimo
                || type == k_mezzo_staccato
                || type == k_mezzo_staccatissimo
                || type == k_staccato
                || type == k_staccato_duro
                || type == k_staccatissimo_duro
                || type == k_staccatissimo
                || type == k_tenuto
                //stress articulations
                || type == k_stress
                || type == k_unstress
                //jazz pitch articulations
                || type == k_scoop
                || type == k_plop
                || type == k_doit
                || type == k_falloff
                //breath marks
                || type == k_breath_mark
                || type == k_caesura
                ;
    }

    //-----------------------------------------------------------------------------------
    //@ soAttachment : { `text` | `textbox` | `line` }
    //@
    bool is_staffobj_attachment(int type)
    {
        return     type == k_text
                || type == k_textbox
                || type == k_line
                ;
    }

    //-----------------------------------------------------------------------------------
    ImoInlineLevelObj* analyse_inline_object()
    {
        // { <inlineWrapper> | <link> | <textItem> | <image> | <button> }

        if(more_params_to_analyse())
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (   /*type == k_inlineWrapper
                ||*/ type == k_txt
                || type == k_image
                || type == k_link
               )
            {
                return static_cast<ImoInlineLevelObj*>(
                    m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr) );
            }
            else
                error_invalid_param();

            move_to_next_param();
        }
        return nullptr;
    }

    //-----------------------------------------------------------------------------------
    void analyse_optional_style(ImoContentObj* pParent)
    {
        // [<style>]
        ImoStyle* pStyle = nullptr;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pParent->set_style(pStyle);
    }

    //-----------------------------------------------------------------------------------
    void analyse_inline_objects(ImoInlinesContainer* pParent)
    {
        // <inlineObject>*
        while( more_params_to_analyse() )
        {
            ImoInlineLevelObj* pItem = analyse_inline_object();
            if (pItem)
                pParent->add_item(pItem);

            move_to_next_param();
        }
    }

    //-----------------------------------------------------------------------------------
    void analyse_inline_or_block_objects(ImoBlocksContainer* pParent)
    {
        // {<inlineObject> | <blockObject>}*
        while (more_params_to_analyse())
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();

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
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pParent);
            }
            else
                error_invalid_param();

            move_to_next_param();
        }
    }

};

//---------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullAnalyser : public ElementAnalyser
{
public:
    NullAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
        string name = m_pLdpFactory->get_name( m_pAnalysedNode->get_type() );
        m_reporter << "Missing analyser for element '" << name << "'. Node ignored." << endl;
    }
};

//@--------------------------------------------------------------------------------------
//@ <anchorLine> = (anchorLine <destination-point>[<lineStyle>][<color>][<width>]
//@                            [<lineCapEnd>])
//@ <destination-point> = <location>    in Tenths
//@     i.e.: (anchorLine (dx value)(dy value)(lineStyle value)(color value)(width value))
//@

class AnchorLineAnalyser : public ElementAnalyser
{
public:
    AnchorLineAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLineStyle* pLine = static_cast<ImoLineStyle*>(
                                    ImFactory::inject(k_imo_line_style, pDoc) );
        pLine->set_start_point( TPoint(0.0f, 0.0f) );
        pLine->set_start_edge(k_edge_normal);
        pLine->set_start_cap(k_cap_none);
        pLine->set_end_edge(k_edge_normal);

        // <destination-point> = <dx><dy>
        TPoint point = TPoint(0.0f, 0.0f);
        if (get_mandatory(k_dx))
            point.x = get_location_param();
        if (get_mandatory(k_dy))
            point.y = get_location_param();
        pLine->set_end_point( point );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            pLine->set_line_style( get_line_style_param() );

        // [<color>])
        if (get_optional(k_color))
            pLine->set_color( get_color_param() );

        // [<width>]
        if (get_optional(k_width))
            pLine->set_width( get_width_param(1.0f) );

        // [<lineCapEnd>])
        if (get_optional(k_lineCapEnd))
            pLine->set_end_cap( get_line_cap_param() );

        add_to_model( pLine );
    }

};

//@--------------------------------------------------------------------------------------
//@ <accentMark> = (<accentType> [<placement>] <printOptions>* )
//@ <accentType> = { accent | legato-duro | marccato | marccato-legato |
//@                  marccato-staccato | mezzo-staccato | staccato |
//@                  staccato-duro | staccatissimo | tenuto }
//@ <stressMark> : (<stressType>` [<placement>] <printOptions>* )
//@ <stressType> : { stress | unstress }
//@
//@ supported but no glyphs. Therefore removed in LDP documentation:
//@     marccato_staccatissimo:    symbol > with black triangle under it
//@     mezzo_staccatissimo:       symbol - with black triangle under it
//@     staccatissimo_duro:        symbol ^ with black triangle under it
//@
//@ <placement> = { above | below }

class AccentAnalyser : public ElementAnalyser
{
public:
    AccentAnalyser(LdpAnalyser* pAnalyser, ostream& reporter,
                         LibraryScope& libraryScope, ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoArticulationSymbol* pImo = static_cast<ImoArticulationSymbol*>(
            ImFactory::inject(k_imo_articulation_symbol, pDoc, get_node_id()) );

        pImo->set_articulation_type( get_accent_type() );

        // <placement>
        if (get_optional(k_label))
            pImo->set_placement( get_placement(k_placement_above) );

        // [<printOptions>*]
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        add_to_model(pImo);
    }

protected:

    int get_accent_type()
    {
        switch (m_pAnalysedNode->get_type())
        {
            //accents
            case k_accent:                  return k_articulation_accent;
            case k_legato_duro:             return k_articulation_legato_duro;
            case k_marccato:                return k_articulation_marccato;
            case k_marccato_legato:         return k_articulation_marccato_legato;
            case k_marccato_staccato:       return k_articulation_marccato_staccato;
            case k_marccato_staccatissimo:  return k_articulation_marccato_staccatissimo;
            case k_mezzo_staccato:          return k_articulation_mezzo_staccato;
            case k_mezzo_staccatissimo:     return k_articulation_mezzo_staccatissimo;
            case k_staccato:                return k_articulation_staccato;
            case k_staccato_duro:           return k_articulation_staccato_duro;
            case k_staccatissimo_duro:      return k_articulation_staccatissimo_duro;
            case k_staccatissimo:           return k_articulation_staccatissimo;
            case k_tenuto:                  return k_articulation_tenuto;
            //stress
            case k_stress:                  return k_articulation_stress;
            case k_unstress:                return k_articulation_unstress;
            default:
                stringstream s;
                s << "Lomse code error: element '" << m_pAnalysedNode->get_name()
                  << "' is not supported in AccentAnalyser." << endl;
                LOMSE_LOG_ERROR(s.str());
                return k_articulation_accent;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <barline> = (barline) | (barline <type>[middle][<visible>][<location>]
//@                                  <soAttachment>*)
//@ <type> = label: { start | end | double | simple | startRepetition |
//@                   endRepetition | doubleRepetition }

class BarlineAnalyser : public ElementAnalyser
{
public:
    BarlineAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc, get_node_id()) );
        pBarline->set_type(k_barline_simple);

        // <type> (label)
        if (get_optional(k_label))
        {
            int type = get_barline_type();
            pBarline->set_type(type);
            if (type == k_barline_double_repetition || type == k_barline_end_repetition)
                pBarline->set_num_repeats(1);
        }

        // [middle] (label)
        if (get_optional(k_label))
        {
            string label = m_pParamToAnalyse->get_value();
            if (label == "middle" )
                pBarline->set_middle(true);
            else
                error_invalid_param();
        }

        // [<visible>][<location>]
        analyse_staffobjs_options(pBarline);

        // <soAttachments>*
        analyse_staffobj_attachments(pBarline);

        error_if_more_elements();

        add_to_model(pBarline);
        add_measure_info(pBarline);
    }

protected:

    int get_barline_type()
    {
        string value = m_pParamToAnalyse->get_value();
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown barline type '" + value + "'. 'simple' barline assumed.");
        }

        return type;
    }

    void add_measure_info(ImoBarline* pBarline)
    {
        TypeMeasureInfo* pInfo = m_pAnalyser->get_measure_info();
        if (pInfo)  //In Unit Tests it could not exist
        {
            pBarline->set_measure_info(pInfo);
            m_pAnalyser->set_measure_info(nullptr);
        }
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


class BeamAnalyser : public ElementAnalyser
{
public:
    BeamAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBeamDto* pInfo = static_cast<ImoBeamDto*>(
                ImFactory::inject(k_imo_beam_dto, pDoc, get_node_id()) );
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );

        // num
        if (get_optional(k_number))
            pInfo->set_beam_number( get_integer_value(0) );
        else
        {
            error_msg("Missing or invalid beam number. Beam ignored.");
            delete pInfo;
            return;
        }

        // <beamtype> (label)
        if (!get_optional(k_label) || !set_beam_type(pInfo))
        {
            error_msg("Missing or invalid beam type. Beam ignored.");
            delete pInfo;
            return;
        }

        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    bool set_beam_type(ImoBeamDto* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
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

class BezierAnalyser : public ElementAnalyser
{
public:
    BezierAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBezierInfo* pBezier = static_cast<ImoBezierInfo*>(
                ImFactory::inject(k_imo_bezier_info, pDoc, get_node_id()) );

        while (more_params_to_analyse())
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
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pBezier);
    }

    void set_x_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        float value = get_float_value(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.x = value;
    }

    void set_y_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        float value = get_float_value(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.y = value;
    }
};

//@--------------------------------------------------------------------------------------
//@ <border> = (border <width><lineStyle><color>)
//@     i.e.: (border (width 2.5)(lineStyle solid)(color #ff0000))

class BorderAnalyser : public ElementAnalyser
{
public:
    BorderAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoBorderDto* border = LOMSE_NEW ImoBorderDto();

        // <width>
        if (get_mandatory(k_width))
            border->set_width( get_width_param(1.0f) );

        // <lineStyle>
        if (get_mandatory(k_lineStyle))
            border->set_style( get_line_style_param() );

        // <color>
        if (get_mandatory(k_color))
            border->set_color( get_color_param() );

        add_to_model(border);
    }

protected:

        void set_box_border(ImoTextBlockInfo& box)
        {
            ImoObj* pImo = proceed(k_border, nullptr);
            if (pImo)
            {
                if (pImo->is_border_dto())
                    box.set_border( static_cast<ImoBorderDto*>(pImo) );
                delete pImo;
            }
        }

};

//@--------------------------------------------------------------------------------------
//@ <breathMark> and <caesura> elements:
//@
//@ <breathMark> = (breathMark [<breathSymbol>] <printOptions>* )
//@ <caesura> = (caesura [<caesuraSymbol>] <printOptions>* )
//@ <breathSymbol> = { comma | tick | V | salzedo }     default 'comma'
//@ <caesuraSymbol> = { normal | thick | short | curved }     default 'normal'

class BreathMarkAnalyser : public ElementAnalyser
{
public:
    BreathMarkAnalyser(LdpAnalyser* pAnalyser, ostream& reporter,
                         LibraryScope& libraryScope, ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoArticulationSymbol* pImo = static_cast<ImoArticulationSymbol*>(
            ImFactory::inject(k_imo_articulation_symbol, pDoc, get_node_id()) );

        pImo->set_articulation_type( get_breath_type() );

        // <symbol>
        if (get_optional(k_label))
            pImo->set_symbol( get_symbol() );

        // [<printOptions>*]
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        add_to_model(pImo);
    }

protected:

    int get_breath_type()
    {
        switch (m_pAnalysedNode->get_type())
        {
            case k_breath_mark:     return k_articulation_breath_mark;
            case k_caesura:         return k_articulation_caesura;
            default:
                stringstream s;
                s << "Lomse code error: element '" << m_pAnalysedNode->get_name()
                  << "' is not supported in BreathMarkAnalyser." << endl;
                LOMSE_LOG_ERROR(s.str());
                return k_articulation_accent;
        }
    }

    int get_symbol()
    {
        string value = m_pParamToAnalyse->get_value();
        int element = m_pAnalysedNode->get_type();

        if (element == k_breath_mark)
        {
            if (value == "comma")
                return ImoArticulationSymbol::k_breath_comma;
            else if (value == "tick")
                return ImoArticulationSymbol::k_breath_tick;
            else if (value == "V")
                return ImoArticulationSymbol::k_breath_v;
            else if (value == "salzedo")
                return ImoArticulationSymbol::k_breath_salzedo;
        }

        else if (element == k_caesura)
        {
            if (value == "normal")
                return ImoArticulationSymbol::k_caesura_normal;
            else if (value == "thick")
                return ImoArticulationSymbol::k_caesura_thick;
            else if (value == "short")
                return ImoArticulationSymbol::k_caesura_short;
            else if (value == "curved")
                return ImoArticulationSymbol::k_caesura_curved;
        }

        stringstream s;
        s << "Symbol '" << value << "' not supported. Ignored." << endl;
        error_msg(s.str());
        return ImoArticulationSymbol::k_default;
    }
};

//@--------------------------------------------------------------------------------------
//@ <chord> = (chord <note>+ )

class ChordAnalyser : public ElementAnalyser
{
public:
    ChordAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoChord* pChord = static_cast<ImoChord*>(
                              ImFactory::inject(k_imo_chord, pDoc, get_node_id()) );

        // <note>*
        while (more_params_to_analyse())
        {
            if (!analyse_optional(k_note, pChord))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_notes_to_music_data(pChord);
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
//@ <clef> = (clef <type> [<symbolSize>] [<staffNum>] [<visible>] [<location>]
//@                <soAttachments>* )
//@ <type> = label: { G | F4 | F3 | C1 | C2 | C3 | C4 | percussion |
//@                   C5 | F5 | G1 | 8_G | G_8 | 8_F4 | F4_8 |
//@                   15_G | G_15 | 15_F4 | F4_15 }
//@ <symbolSize> = (symbolSize { full | cue | large } )

class ClefAnalyser : public ElementAnalyser
{
public:
    ClefAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>(
                            ImFactory::inject(k_imo_clef, pDoc, get_node_id()) );

        // <type> (label)
        if (get_optional(k_label))
            pClef->set_clef_type( get_clef_type() );

        // [<symbolSize>]
        if (get_optional(k_symbolSize))
            set_symbol_size(pClef);

        // [<staffNum>][visible][<location>]
        analyse_staffobjs_options(pClef);

        // <soAttachments>*
        analyse_staffobj_attachments(pClef);

        error_if_more_elements();

        //set values that can be inherited
        pClef->set_staff( m_pAnalyser->get_current_staff() );

        add_to_model(pClef);
    }

 protected:

    int get_clef_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int clef = LdpAnalyser::ldp_name_to_clef_type(value);
        if (clef == k_clef_undefined)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown clef type '" + value + "'. Assumed 'G'.");
            return k_clef_G2;
        }
        else
            return clef;
    }

    void set_symbol_size(ImoClef* pClef)
    {
        const std::string& value = m_pParamToAnalyse->get_parameter(1)->get_value();
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

class ColorAnalyser : public ElementAnalyser
{
public:
    ColorAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {

        if (!get_optional(k_label) || !set_color())
        {
            error_msg("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
            return;
        }

        error_if_more_elements();
    }

protected:

    bool set_color()
    {
        ImoColorDto* pColor = LOMSE_NEW ImoColorDto();
        m_pAnalysedNode->set_imo(pColor);
        std::string value = get_string_value();
        pColor->set_from_string(value);
        return pColor->is_ok();
    }

};

//@--------------------------------------------------------------------------------------
//@ <content> = (content { <heading> | <dynamic> | <itemizedlist> | <orderedlist> |
//@                        <para> | <score> | <scorePlayer> | <table> | <text> }* )

class ContentAnalyser : public ElementAnalyser
{
public:
    ContentAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                        ImFactory::inject(k_imo_content, pDoc, get_node_id()) );

        //node must be added to model before adding content to it, because
        //dynamic objects will generate requests to create other obejcts
        //on the fly, and this reuires taht all nodes are properly chained
        //in the tree.
        add_to_model(pContent);

        while (more_params_to_analyse())
        {
            if (! (analyse_optional(k_score, pContent)
                 || analyse_optional(k_dynamic, pContent)
                 || analyse_optional(k_heading, pContent)
                 || analyse_optional(k_itemizedlist, pContent)
                 || analyse_optional(k_orderedlist, pContent)
                 || analyse_optional(k_para, pContent)
                 || analyse_optional(k_score_player, pContent)
                 || analyse_optional(k_table, pContent)
                 || analyse_optional(k_text, pContent)
               ))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        error_if_more_elements();
    }
};

//@--------------------------------------------------------------------------------------
//@ <newSystem> = (newSystem}

class ControlAnalyser : public ElementAnalyser
{
public:
    ControlAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemBreak* pCtrl = static_cast<ImoSystemBreak*>(
                        ImFactory::inject(k_imo_system_break, pDoc, get_node_id()) );
        add_to_model(pCtrl);
    }
};

//@--------------------------------------------------------------------------------------
//@ <cursor> = (cursor <instrNumber><staffNumber><timePos><objID>)
//@ <instrNumber> = integer number (0..n-1)
//@ <staffNumber> = integer number (0..n-1)
//@ <timePos> = float number
//@ <objID> = integer number
//@

class CursorAnalyser : public ElementAnalyser
{
public:
    CursorAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoCursorInfo* pCursor = static_cast<ImoCursorInfo*>(
                        ImFactory::inject(k_imo_cursor_info, pDoc, get_node_id()));

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
    }
};

//@--------------------------------------------------------------------------------------
//@ <defineStyle> = (defineStyle <syleName> { <styleProperty>* | <font>[<color>] } )
//@ <styleProperty> = (property-tag value)
//@
//@ Examples:
//@     (defineStyle "Composer" (font "Times New Roman" 12pt normal) (color #000000))
//@     (defineStyle "Instruments" (font "Times New Roman" 14pt bold))
//@     (defineStyle "para"
//@         (font-name "Times New Roman")
//@         (font-size 12pt)
//@         (font-style normal)
//@         (color #ff0000)
//@         (margin-bottom 2em)
//@     )
//@
//@ font-style :  { "normal" | "italic" }
//@ font-weight : { "normal" | "bold" }
//@

class DefineStyleAnalyser : public ElementAnalyser
{
public:
    DefineStyleAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoStyle* pStyle;
        string name;

        //<styleName>
        if (get_mandatory(k_string))
            name = get_string_value();
        else
            return;
        string parent = (name == "Default style" ? "" : "Default style");

        pStyle = create_style(name, parent);

        // <font>[<color>]
        if (analyse_optional(k_font, pStyle))
        {
            //[<color>]
            if (get_optional(k_color))
                pStyle->color( get_color_param() );
            else
                pStyle->color( Color(0,0,0) );
        }
        else
        {
            // <styleProperty>*
            bool fHasFontFile = false;
            while (more_params_to_analyse())
            {
                // color and background
                if (get_optional(k_color))
                    pStyle->color( get_color_param() );
                else if (get_optional(k_background_color))
                    pStyle->background_color( get_color_param() );

                // font
                else if (get_optional(k_font_file))
                {
                    pStyle->font_file( get_string_param() );
                    fHasFontFile = true;
                }
                else if (get_optional(k_font_name))
                {
                    pStyle->font_name( get_string_param() );
                    if (!fHasFontFile)
                        pStyle->font_file("");
                }
                else if (get_optional(k_font_size))
                    pStyle->font_size( get_font_size_param() );
                else if (get_optional(k_font_style))
                    pStyle->font_style( get_font_style() );
                else if (get_optional(k_font_weight))
                    pStyle->font_weight( get_font_weight() );

//                // border
//                else if (get_optional(k_border))
//                    pStyle->border( get_lenght_param() );
//                else if (get_optional(k_border_top))
//                    pStyle->border_top( get_lenght_param() );
//                else if (get_optional(k_border_right))
//                    pStyle->border_right( get_lenght_param() );
//                else if (get_optional(k_border_bottom))
//                    pStyle->border_bottom( get_lenght_param() );
//                else if (get_optional(k_border_left))
//                    pStyle->border_left( get_lenght_param() );

                // border width
                else if (get_optional(k_border_width))
                    pStyle->border_width( get_lenght_param() );
                else if (get_optional(k_border_width_top))
                    pStyle->border_width_top( get_lenght_param() );
                else if (get_optional(k_border_width_right))
                    pStyle->border_width_right( get_lenght_param() );
                else if (get_optional(k_border_width_bottom))
                    pStyle->border_width_bottom( get_lenght_param() );
                else if (get_optional(k_border_width_left))
                    pStyle->border_width_left( get_lenght_param() );

                // margin
                else if (get_optional(k_margin))
                    pStyle->margin( get_lenght_param() );
                else if (get_optional(k_margin_top))
                    pStyle->margin_top( get_lenght_param() );
                else if (get_optional(k_margin_right))
                    pStyle->margin_right( get_lenght_param() );
                else if (get_optional(k_margin_bottom))
                    pStyle->margin_bottom( get_lenght_param() );
                else if (get_optional(k_margin_left))
                    pStyle->margin_left( get_lenght_param() );

                // padding
                else if (get_optional(k_padding))
                    pStyle->padding( get_lenght_param() );
                else if (get_optional(k_padding_top))
                    pStyle->padding_top( get_lenght_param() );
                else if (get_optional(k_padding_right))
                    pStyle->padding_right( get_lenght_param() );
                else if (get_optional(k_padding_bottom))
                    pStyle->padding_bottom( get_lenght_param() );
                else if (get_optional(k_padding_left))
                    pStyle->padding_left( get_lenght_param() );

                //text
                else if (get_optional(k_text_decoration))
                    pStyle->text_decoration( get_text_decoration() );
                else if (get_optional(k_vertical_align))
                    pStyle->vertical_align( get_valign() );
                else if (get_optional(k_text_align))
                    pStyle->text_align( get_text_align() );
                else if (get_optional(k_line_height))
                    pStyle->line_height( get_float_param(1.5f) );

                //size
                else if (get_optional(k_min_height))
                    pStyle->min_height( get_lenght_param() );
                else if (get_optional(k_min_width))
                    pStyle->min_width( get_lenght_param() );
                else if (get_optional(k_max_height))
                    pStyle->max_height( get_lenght_param() );
                else if (get_optional(k_max_width))
                    pStyle->max_width( get_lenght_param() );
                else if (get_optional(k_height))
                    pStyle->height( get_lenght_param() );
                else if (get_optional(k_width))
                    pStyle->width( get_lenght_param() );

                //table
                else if (get_optional(k_table_col_width))
                    pStyle->table_col_width( get_lenght_param() );

                else
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
        }

        error_if_more_elements();

        if (pStyle->get_name() != "Default style")
            add_to_model(pStyle);
    }

protected:

    int get_font_style()
    {
        const string value = get_string_param();
        if (value == "normal")
            return ImoStyle::k_font_style_normal;
        else if (value == "italic")
            return ImoStyle::k_font_style_italic;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font-style '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_style_normal;
        }
    }

    int get_font_weight()
    {
        const string value = get_string_param();
        if (value == "normal")
            return ImoStyle::k_font_weight_normal;
        else if (value == "bold")
            return ImoStyle::k_font_weight_bold;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font-weight '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_weight_normal;
        }
    }

    int get_text_decoration()
    {
        const string value = get_string_param();
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown text decoration value '" + value + "'. Replaced by 'none'.");
            return ImoStyle::k_decoration_none;
        }
    }

    int get_valign()
    {
        const string value = get_string_param();
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown vertical align '" + value + "'. Replaced by 'baseline'.");
            return ImoStyle::k_valign_baseline;
        }
    }

    int get_text_align()
    {
        const string value = get_string_param();
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
            report_msg(m_pParamToAnalyse->get_line_number(),
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
            ImoStyle* pStyle = static_cast<ImoStyle*>(
                                  ImFactory::inject(k_imo_style, pDoc, get_node_id()));
            pStyle->set_name(name);
            pStyle->set_parent_style(pParent);
            return pStyle;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ ImoDirection StaffObj
//@ <direction> = (dir <staffobjOptions>* <dirAttachments>*)
//@ dirAttachment : { `metronome` }
//@
class DirectionAnalyser : public ElementAnalyser
{
public:
    DirectionAnalyser(LdpAnalyser* pAnalyser, ostream& reporter,
                      LibraryScope& libraryScope, ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pDir = static_cast<ImoDirection*>(
                    ImFactory::inject(k_imo_direction, pDoc, get_node_id()) );

        // <staffobjOptions>*
        analyse_staffobjs_options(pDir);

        // <dirAttachments>*
        analyse_attachments(pDir);

        add_to_model(pDir);
    }

protected:

    void analyse_attachments(ImoDirection* pDir)
    {
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (type == k_metronome)
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pDir);
            else
                error_invalid_param();

            move_to_next_param();
        }
    }
};

//@--------------------------------------------------------------------------------------
//@ For dynamic content, i.e. exercises
//@
//@ <dynamic> = (dynamic <classid> <param>*)
//@ <classid> = (classid <label>)
//@ <param> = (param <name><value>)
//@ <name> = <label>
//@ <value> = <string>
//@
//@ Example:
//@  (dynamic
//@      (classid IdfyCadences) width="100%" height="300" border="0">
//@      (param cadences "all")
//@      (param cadence_buttons "terminal,transient")
//@      (param mode "earTraining")
//@  )
//@

class DynamicAnalyser : public ElementAnalyser
{
public:
    DynamicAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamic* pDyn = static_cast<ImoDynamic*>(
                            ImFactory::inject(k_imo_dynamic, pDoc, get_node_id()) );

        // <classid>
        if (get_mandatory(k_classid))
            set_classid(pDyn);

        // [<param>*]
        while (analyse_optional(k_parameter, pDyn));

        error_if_more_elements();

        //ImoDynamic must be included in model before asking to create its content
        add_to_model(pDyn);

        //ask user app to generate content for this dynamic object
        RequestDynamic request(pDoc, pDyn);
        m_libraryScope.post_request(&request);
    }

protected:

    void set_classid(ImoDynamic* pDyn)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        pDyn->set_classid( get_string_value() );
    }
};

//@--------------------------------------------------------------------------------------
//@ <dynamics> = (dyn string [<placement>] <printOptions>* )
//@ <placement> = { above | below }

class DynamicsAnalyser : public ElementAnalyser
{
public:
    DynamicsAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // <string>
        if (!get_mandatory(k_string))
            return;

        string type = get_string_value();
        if (!is_valid_dynamics(type))
            return;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(
                                ImFactory::inject(k_imo_dynamics_mark, pDoc, get_node_id()) );
        pImo->set_mark_type(type);

        // [<placement>]
        if (get_optional(k_label))
            pImo->set_placement( get_placement(k_placement_above) );

        // [<printOptions>]
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        add_to_model(pImo);
    }

protected:

    bool is_valid_dynamics(const string& type)
    {
        if (type == "p")            return true;
        else if (type == "m")       return true;
        else if (type == "f")       return true;
        else if (type == "r")       return true;
        else if (type == "s")       return true;
        else if (type == "z")       return true;
        else if (type == "n")       return true;
        else if (type == "pppppp")  return true;
        else if (type == "ppppp")   return true;
        else if (type == "pppp")    return true;
        else if (type == "ppp")     return true;
        else if (type == "pp")      return true;
        else if (type == "mp")      return true;
        else if (type == "mf")      return true;
        else if (type == "pf")      return true;
        else if (type == "ff")      return true;
        else if (type == "fff")     return true;
        else if (type == "ffff")    return true;
        else if (type == "fffff")   return true;
        else if (type == "ffffff")  return true;
        else if (type == "fp")      return true;
        else if (type == "fz")      return true;
        else if (type == "sf")      return true;
        else if (type == "sfp")     return true;
        else if (type == "sfpp")    return true;
        else if (type == "sfz")     return true;
        else if (type == "sfzp")    return true;
        else if (type == "sffz")    return true;
        else if (type == "rf")      return true;
        else if (type == "rfz")     return true;

        stringstream s;
        s << "Dynamics string '" << type
          << "' not supported. <dyn> ignored." << endl;
        error_msg(s.str());

        return false;
    }

};

//@--------------------------------------------------------------------------------------
//@ <fermata> = (fermata [<fermataSymbol>] [<placement>] <printOptions>* )
//@ <fermataSymbol> = { normal | short | long | very-short | very-long |
//@                     henze-short | henze-long }     default 'normal'
//@ <placement> = { above | below }

class FermataAnalyser : public ElementAnalyser
{
public:
    FermataAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoFermata* pImo = static_cast<ImoFermata*>(
                                ImFactory::inject(k_imo_fermata, pDoc, get_node_id()) );

        // [<fermataSymbol>] [<placement>]
        if (get_optional(k_label))
        {
            string label = m_pParamToAnalyse->get_value();
            int symbol;
            int placement;
            if (get_symbol(label, &symbol))
            {
                pImo->set_symbol(symbol);

                // [<placement>]
                if (get_optional(k_label))
                    pImo->set_placement(
                            ElementAnalyser::get_placement(k_placement_above) );
            }
            else if (get_placement(label, &placement))
            {
                pImo->set_placement(placement);
            }
            else
            {
                stringstream s;

                //try [<placement>] after invalid symbol
                if (get_optional(k_label))
                {
                    pImo->set_placement(
                            ElementAnalyser::get_placement(k_placement_above) );

                    //report invalid symbol
                    s << "Symbol '" << label << "' not supported. Ignored.";
                }
                else
                {
                    //report invalid parameter
                    s << "Parameter '" << label << "' not supported. Ignored.";
                }
                error_msg(s.str());
            }
        }

        // [<printOptions>*]
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        add_to_model(pImo);
    }

protected:

    bool get_symbol(const string& value, int* symbol)
    {
        if (value == "short")
        {
            *symbol = ImoFermata::k_short;
            return true;
        }
        else if (value == "long")
        {
            *symbol = ImoFermata::k_long;
            return true;
        }
        else if (value == "henze-short")
        {
            *symbol = ImoFermata::k_henze_short;
            return true;
        }
        else if (value == "henze-long")
        {
            *symbol = ImoFermata::k_henze_long;
            return true;
        }
        else if (value == "very-short")
        {
            *symbol = ImoFermata::k_very_short;
            return true;
        }
        else if (value == "very-long")
        {
            *symbol = ImoFermata::k_very_long;
            return true;
        }
        else if (value == "normal")
        {
            *symbol = ImoFermata::k_normal;
            return true;
        }
        return false;
    }

    bool get_placement(const string& value, int* placement)
    {
        if (value == "above")
        {
            *placement = k_placement_above;
            return true;
        }
        else if (value == "below")
        {
            *placement = k_placement_below;
            return true;
        }
        return false;
    }

};

//@--------------------------------------------------------------------------------------
//@ <figuredBass> = (figuredBass <figuredBassSymbols>[<parenthesis>][<fbline>]
//@                              [<staffObjOptions>*] )
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

class FiguredBassAnalyser : public ElementAnalyser
{
public:
    FiguredBassAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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

            // [<staffObjOptions>*]
            //analyse_scoreobj_options(dto);

            error_if_more_elements();

            add_to_model( LOMSE_NEW ImoFiguredBass(info) );
        }
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
////                            sElmName.c_str()() );
////            else
////                pFBLineInfo[nFBL++] = AnalyzeFBLine(pX, pVStaff);
////        }
////        else
////            AnalysisError(pX, "[Element '%s'. Invalid parameter '%s'. Ignored.",
////                          sElmName.c_str(), sName.c_str()() );
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


class FontAnalyser : public ElementAnalyser
{
public:
    FontAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
    }

    void set_font_style_weight(ImoFontStyleDto* pFont)
    {
        const string& value = m_pParamToAnalyse->get_value();
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font style '" + value + "'. Replaced by 'normal'.");
            pFont->weight = ImoStyle::k_font_weight_normal;
            pFont->style = ImoStyle::k_font_style_normal;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <goFwd> = (goFwd <duration> [voice])
//@ <duration> = note/rest duration, letter plus dots, i.e. 'e..'
//@
//@ Version 1.x
//@ <goBack> = (goBack <timeShift>)
//@ <goFwd> = (goFwd <timeShift>)
//@ <timeShift> = { start | end | <number> | <duration> }
//@
//@ the time shift can be:
//@   a) one of the tags 'start' and 'end': i.e. (goBack start) (goFwd end)
//@   b) a number: the amount of 256th notes to go forward or backwards
//@   c) a note/rest duration, i.e. 'e..'

class GoBackFwdAnalyser : public ElementAnalyser
{
public:
    GoBackFwdAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        if (m_pAnalyser->get_score_version() < 200)
            do_analysis_v1();
        else
            do_analysis_v2();
    }

protected:

    void do_analysis_v1()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                ImFactory::inject(k_imo_go_back_fwd, pDoc, get_node_id()) );
        bool fFwd = m_pAnalysedNode->is_type(k_goFwd);
        pImo->set_forward(fFwd);

        // <duration> | start | end (label) or <number>
        if (get_optional(k_label))
        {
            string duration = m_pParamToAnalyse->get_value();
            if (duration == "start")
            {
                if (!fFwd)
                    pImo->set_to_start();
                else
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Element 'goFwd' has an incoherent value: go forward to start?. Element ignored.");
                    delete pImo;
                    return;
                }
            }
            else if (duration == "end")
            {
                if (fFwd)
                    pImo->set_to_end();
                else
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Element 'goBack' has an incoherent value: go backwards to end?. Element ignored.");
                    delete pImo;
                    return;
                }
            }
            else
            {
                NoteTypeAndDots figdots = ldp_duration_to_components(duration);
                if (figdots.noteType == k_unknown_notetype)
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Unknown duration '" + duration + "'. Element ignored.");
                    delete pImo;
                    return;
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
            float rTime = m_pParamToAnalyse->get_value_as_float();
            if (rTime < 0.0f)
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                    "Negative value for element 'goFwd/goBack'. Element ignored.");
                delete pImo;
                return;
            }
            else
                pImo->set_time_shift(rTime);
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown duration '" + m_pParamToAnalyse->get_name() + "'. Element ignored.");
            delete pImo;
            return;
        }

        error_if_more_elements();

        add_to_model(pImo);
    }

    //-----------------------------------------------------------------------------------
    void do_analysis_v2()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoRest* pImo = static_cast<ImoRest*>(
                                ImFactory::inject(k_imo_rest, pDoc, get_node_id()) );
        pImo->mark_as_go_fwd();
        pImo->set_visible(false);

        // <duration> (label)
        //AWARE: As goFwd is a rest, only note/rest duration is allowed (i.e. "e.")
        //       Duration for goFwd is no longer alloed as number (i.e. 32)
        if (get_mandatory(k_label))
            set_duration(pImo);

        // [{ <voice> | <staffNum> }*]
        analyse_options(pImo);

        // [<staffObjOptions>*]
        analyse_staffobjs_options(pImo);

        pImo->set_staff( m_pAnalyser->get_current_staff() );
        pImo->set_voice( m_pAnalyser->get_current_voice() );

        add_to_model(pImo);
    }

    //-----------------------------------------------------------------------------------
    void set_duration(ImoNoteRest* pNR)
    {
        NoteTypeAndDots figdots = get_note_type_and_dots();
        pNR->set_note_type_and_dots(figdots.noteType, figdots.dots);
    }

    //-----------------------------------------------------------------------------------
    void analyse_options(ImoNoteRest* pNR)
    {
        // { <voice> | <staffNum> }

        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (type == k_voice)
            {
                set_voice_element(pNR);
            }
            else if (type == k_staffNum)
            {
                set_staff_num_element(pNR);
            }
            else if (type == k_label)   //possible staffNum as 'p1' or voice as 'v2'
            {
                char first = (m_pParamToAnalyse->get_value())[0];
                if (first == 'p')
                    get_num_staff();
                else if (first == 'v')
                    get_voice();
                else
                    error_invalid_param();
            }
            else
                break;

            move_to_next_param();
        }
    }

    void set_voice_element(ImoNoteRest* pNR)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int voice = get_integer_value( m_pAnalyser->get_current_voice() );
        m_pAnalyser->set_current_voice(voice);
        pNR->set_voice(voice);
    }

    void set_staff_num_element(ImoNoteRest* pNR)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int curStaff = m_pAnalyser->get_current_staff() + 1;
        int staff = get_integer_value(curStaff) - 1;
        m_pAnalyser->set_current_staff(staff);
        pNR->set_staff(staff);
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

class GraphicAnalyser : public ElementAnalyser
{
public:
    GraphicAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // "line"
        if (get_optional(k_label))
        {
            string value = m_pParamToAnalyse->get_value();
            if (value != "line")
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Unknown type '" + value + "'. Element 'graphic' ignored.");
                return;
            }
        }

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>(
                            ImFactory::inject(k_imo_score_line, pDoc, get_node_id()));

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
    }

};
#endif  //LOMSE_COMPATIBILITY_LDP_1_5

//@--------------------------------------------------------------------------------------
//@ <group> = (group firstInstrId lastInstrId [<name>][<abbrev>][<symbol>]
//@                  [<joinBarlines>] )
//@
//@ <name> = <textString>
//@ <abbrev> = <textString>
//@ <symbol> = (symbol {none | brace | bracket | line} )
//@ <joinBarlines> = (joinBarlines {yes | no | mensurstrich } )

class GroupAnalyser : public ElementAnalyser
{
public:
    GroupAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();

        // firstInstrId
        ImoInstrument* pFirstInstr = nullptr;
        if (get_mandatory(k_label))
        {
            string partId = m_pParamToAnalyse->get_value();
            pFirstInstr = pScore->get_instrument(partId);
//            if (pFirstInstr == nullptr)
//            {
//                error_msg("");
//            }
        }
        else
            return;

        // lastInstrId
        ImoInstrument* pLastInstr = nullptr;
        if (get_mandatory(k_label))
        {
            string partId = m_pParamToAnalyse->get_value();
            pLastInstr = pScore->get_instrument(partId);
//            if (pFirstInstr == nullptr)
//            {
//                error_msg("");
//            }
        }
        else
            return;

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                        ImFactory::inject(k_imo_instr_group, pDoc, get_node_id()));

        // [<name>]
        analyse_optional(k_name, pGrp);

        // [<abbrev>]
        analyse_optional(k_abbrev, pGrp);

        // [<symbol>]
        if (get_optional(k_symbol))
            set_symbol(pGrp);

        // [<joinBarlines>]
        if (get_optional(k_joinBarlines))
            set_join_barlines(pGrp);

        error_if_more_elements();

        add_instruments_to_group(pScore, pGrp, pFirstInstr, pLastInstr);
        add_to_model(pGrp);
    }

protected:

    void set_symbol(ImoInstrGroup* pGrp)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string symbol = get_string_value();
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

    void set_join_barlines(ImoInstrGroup* pGrp)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string value = get_string_value();
        if (value == "yes")
            pGrp->set_join_barlines(ImoInstrGroup::k_standard);
        else if (value == "no")
            pGrp->set_join_barlines(ImoInstrGroup::k_no);
        else if (value == "mensurstrich")
            pGrp->set_join_barlines(ImoInstrGroup::k_mensurstrich);
        else
        {
            pGrp->set_join_barlines(ImoInstrGroup::k_standard);
            error_msg("Invalid value for joinBarlines. Must be "
                      "'yes', 'no' or 'mensurstrich'. 'yes' assumed.");
        }
    }

    void add_instruments_to_group(ImoScore* pScore, ImoInstrGroup* pGrp,
                                  ImoInstrument* pFirstInstr, ImoInstrument* pLastInstr)
    {
        ImoInstruments* pColInstr = pScore->get_instruments();
        ImoObj::children_iterator it;
        bool fAdd = false;
        for (it= pColInstr->begin(); it != pColInstr->end(); ++it)
        {
            ImoInstrument* pInstr = static_cast<ImoInstrument*>(*it);
            if (fAdd)
                pGrp->add_instrument(pInstr);

            if (pInstr == pFirstInstr)
            {
                pGrp->add_instrument(pInstr);
                fAdd = true;
            }
            else if (pInstr == pLastInstr)
            {
                pGrp->add_instrument(pInstr);
                break;
            }
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <heading> = (heading <level> [<style>] <inlineObject>*)

class HeadingAnalyser : public ElementAnalyser
{
public:
    HeadingAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // <level> (num)
        if (get_mandatory(k_number))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoHeading* pHeading = static_cast<ImoHeading*>(
                            ImFactory::inject(k_imo_heading, pDoc, get_node_id()) );
            pHeading->set_level( get_integer_value(1) );

            // [<style>]
            analyse_optional_style(pHeading);

            // <inlineObject>+
            analyse_inline_objects(pHeading);

            add_to_model(pHeading);
        }

    }
};

//@--------------------------------------------------------------------------------------
//@ <image> = (image [<style>] <file>)
//@ <file> = (file <string>)
//@
//@
//@

class ImageAnalyser : public ElementAnalyser
{
public:
    ImageAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoImage* pImg = static_cast<ImoImage*>(
                                ImFactory::inject(k_imo_image, pDoc, get_node_id()) );

        // [<style>]
        analyse_optional_style(pImg);

        // <file>
        if (get_mandatory(k_file))
        {
            LdpElement* pValue = m_pParamToAnalyse->get_first_child();
            load_image(pImg, pValue->get_value(), get_document_locator());
        }

        add_to_model(pImg);
    }

protected:

    void load_image(ImoImage* pImg, string imagename, string locator)
    {
        DocLocator loc(locator);
        SpImage img = ImageReader::load_image( loc.get_locator_for_image(imagename) );
        pImg->set_content(img);
        if (!img->is_ok())
            report_msg(m_pAnalysedNode->get_line_number(), "Error loading image. " + img->get_error_msg());
    }
};

//@--------------------------------------------------------------------------------------
//@ <infoMIDI> = (infoMIDI num_instr [num_channel])
//@ num_instr = integer: 0..127
//@ num_channel = integer: 0..15

class InfoMidiAnalyser : public ElementAnalyser
{
public:
    InfoMidiAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSoundInfo* pInfo = static_cast<ImoSoundInfo*>(
                            ImFactory::inject(k_imo_sound_info, pDoc, get_node_id()) );

        // num_instr
        if (!get_optional(k_number) || !set_midi_program(pInfo))
        {
            error_msg("Missing or invalid MIDI instrument (0..127). MIDI info ignored.");
            delete pInfo;
            return;
        }

        // [num_channel]
        if (get_optional(k_number) && !set_midi_channel(pInfo))
        {
            report_msg(m_pAnalysedNode->get_line_number(),
                        "Invalid MIDI channel (0..15). Channel info ignored.");
        }

        error_if_more_elements();

        add_to_model(pInfo);
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
//@ <instrument> = (instrument [<partId>][<instrName>][<instrAbbrev>][<staves>]
//@                            [<staff>]*[<infoMIDI>] <musicData> )
//@ Note. partId is mandatory if <parts> has been defined.
//@ <instrName> = <textString>
//@ <instrAbbrev> = <textString>

class InstrumentAnalyser : public ElementAnalyser
{
public:
    InstrumentAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        m_pAnalyser->clear_pending_relations();
        m_pAnalyser->reset_defaults_for_instrument();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrument* pInstrument = nullptr;

        // [<instrId>]
        if (get_optional(k_label))
        {
            string partId = m_pParamToAnalyse->get_value();
            ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
            pInstrument = pScore->get_instrument(partId);
            if (pInstrument == nullptr)
            {
                if (m_pAnalyser->is_instr_id_required())
                {
                    error_msg("'partId' is not defined in <parts> element. "
                              "Instrument ignored.");
                    return;
                }
                else
                    pInstrument = static_cast<ImoInstrument*>(
                                ImFactory::inject(k_imo_instrument, pDoc, get_node_id()) );
            }
            pInstrument->set_instr_id(partId);
        }
        else if (m_pAnalyser->is_instr_id_required())
        {
            error_msg("instrument: missing 'partId'. Instrument ignored.");
            return;
        }
        else
            pInstrument = static_cast<ImoInstrument*>(
                            ImFactory::inject(k_imo_instrument, pDoc, get_node_id()) );

        // [<instrName>]
        analyse_optional(k_name, pInstrument);

        // [<instrAbbrev>]
        analyse_optional(k_abbrev, pInstrument);

        // [<staves>]
        if (get_optional(k_staves))
            set_staves(pInstrument);

        // [<staff>]*
        while (analyse_optional(k_staff, pInstrument));

        //FIX: For adding space for lyrics
        m_pAnalyser->set_current_instrument(pInstrument);
        if (!m_pAnalyser->is_instr_id_required())
        {
            pInstrument->reserve_space_for_lyrics(0, m_pAnalyser->m_extraMarginSpace);
            m_pAnalyser->m_extraMarginSpace = 0.0f;
        }

        // [<infoMIDI>]
        analyse_optional(k_infoMIDI, pInstrument);

        // <musicData>
        analyse_mandatory(k_musicData, pInstrument);

        error_if_more_elements();

        if (!m_pAnalyser->is_instr_id_required())
        {
            add_to_model(pInstrument);
            add_sound_info_if_needed(pInstrument);
        }

    }

protected:

    void set_staves(ImoInstrument* pInstrument)
    {
        // <staves> = (staves <num>)

        LdpElement* pValue = m_pParamToAnalyse->get_first_child();
        string staves = pValue->get_value();
        int nStaves;
        bool fError = !pValue->is_type(k_number);
        if (!fError)
        {
            std::istringstream iss(staves);
            fError = (iss >> std::dec >> nStaves).fail();
        }
        if (fError)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid value '" + staves + "' for staves. Replaced by 1.");
        }
        else
        {
            // coverity[tainted_data]
            for(; nStaves > 1; --nStaves)
                pInstrument->add_staff();
        }
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
//@ <key> = (key <type> <staffobjOptions>* <soAttachments>* )
//@ <type> = label: { C | G | D | A | E | B | F+ | C+ | C- | G- | D- | A- |
//@                   E- | B- | F | a | e | b | f+ | c+ | g+ | d+ | a+ | a- |
//@                   e- | b- | f | c | g | d }

class KeySignatureAnalyser : public ElementAnalyser
{
public:
    KeySignatureAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>(
                            ImFactory::inject(k_imo_key_signature, pDoc, get_node_id()) );

        // <type> (label)
        if (get_optional(k_label))
            pKey->set_key_type( get_key_type() );

        // [<staffobjOptions>*]
        analyse_staffobjs_options(pKey);

        // <soAttachments>*
        analyse_staffobj_attachments(pKey);

        error_if_more_elements();

        add_to_model(pKey);
    }

    int get_key_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = LdpAnalyser::ldp_name_to_key_type(value);
        if (type == k_key_undefined)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown key '" + value + "'. Assumed 'C'.");
            type = k_key_C;
        }
        return type;
    }

};

//@--------------------------------------------------------------------------------------
//@ <language> = (language <languageCode>)
class LanguageAnalyser : public ElementAnalyser
{
public:
    LanguageAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
    }
};

//@--------------------------------------------------------------------------------------
//@ <lenmusdoc> = (lenmusdoc <vers>[language][<settings>][<meta>][<styles>]<content>)
//@ <styles> = (styles [<defineStyle>*][<pageLayout>*])

class LenmusdocAnalyser : public ElementAnalyser
{
public:
    LenmusdocAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
        ImoDocument* pImoDoc = nullptr;

        // <vers>
        if (get_mandatory(k_vers))
        {
            string version = get_version();
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            pImoDoc = static_cast<ImoDocument*>(
                                ImFactory::inject(k_imo_document, pDoc, get_node_id()));
            pImoDoc->set_version(version);
            m_pAnalyser->save_root_imo_document(pImoDoc);
            pDoc->set_imo_doc(pImoDoc);
        }
        else
            return;

        // [<language>]
        if (get_optional(k_language))
        {
            string language = get_language();
            pImoDoc->set_language( language );
        }

        // [<settings>]
        analyse_optional(k_settings, pImoDoc);

        // [<meta>]
        analyse_optional(k_meta, pImoDoc);

        // [<styles>]
        if (!analyse_optional(k_styles, pImoDoc))
            add_default(pImoDoc);

        // [<pageLayout>*]
        while (analyse_optional(k_pageLayout, pImoDoc));

        // <content>
        analyse_mandatory(k_content, pImoDoc);

        error_if_more_elements();

        m_pAnalysedNode->set_imo(pImoDoc);
    }

protected:

    string get_version()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
    }

    string get_language()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
    }

    void add_default(ImoDocument* pImoDoc)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        Linker linker(pDoc);
        ImoStyles* pStyles = static_cast<ImoStyles*>(
                        ImFactory::inject(k_imo_styles, pDoc, get_node_id()));
        linker.add_child_to_model(pImoDoc, pStyles, k_styles);
        ImoStyle* pDefStyle = pImoDoc->get_default_style();
        pImoDoc->set_style(pDefStyle);
    }

};

//@--------------------------------------------------------------------------------------
//@ <line> = (line <startPoint><endPoint>[<width>][<color>]
//@                [<lineStyle>][<startCap>][<endCap>])
//@ <width> = (width tenths)    [1.0]
//@ <color> = (color value)    [#000000]  (solid black)
//@ <startPoint> = (startPoint <location>)      (coordinates in tenths)
//@ <endPoint> = (endPoint <location>)      (coordinates in tenths)
//@ <lineStyle> = (lineStyle { none | solid | longDash | shortDash | dot | dotDash } ) [solid]
//@ <startCap> = (lineCapStart <capType>)    [none]
//@ <endCap> = (lineCapEnd <capType>)    [none]
//@ <capType> = label: { none | arrowhead | arrowtail | circle | square | diamond }
//@
//@ Example:
//@     (line (startPoint (dx 5.0)(dy 5.0)) (endPoint (dx 80.0)(dy -10.0))
//@           (width 1.0)(color #000000)(lineStyle solid)
//@           (lineCapStart arrowhead)(lineCapEnd none) )
//@

class LineAnalyser : public ElementAnalyser
{
public:
    LineAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScoreLine* pLine = static_cast<ImoScoreLine*>(
                        ImFactory::inject(k_imo_score_line, pDoc, get_node_id()) );

        // <startPoint>
        if (get_mandatory(k_startPoint))
            pLine->set_start_point( get_point_param() );

        // <endPoint>
        if (get_mandatory(k_endPoint))
            pLine->set_end_point( get_point_param() );

        // [<width>]
        if (get_optional(k_width))
            pLine->set_width( get_width_param(1.0f) );

        // [<color>])
        if (get_optional(k_color))
            pLine->set_color( get_color_param() );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            pLine->set_line_style( get_line_style_param() );

        // [<startCap>]
        if (get_optional(k_lineCapStart))
            pLine->set_start_cap( get_line_cap_param() );

        // [<endCap>]
        if (get_optional(k_lineCapEnd))
            pLine->set_end_cap( get_line_cap_param() );

        add_to_model(pLine);
    }

};

//@--------------------------------------------------------------------------------------
//@ <link> = (link [<style>] <url> <inlineObject>+ )
//@ <url> = (url <strings>)
//@
//@ Example:
//@     (link (url "#TheoryHarmony_ch3.lms")(txt "Harmony exercise"))
//@

class LinkAnalyser : public ElementAnalyser
{
public:
    LinkAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLink* pLink = static_cast<ImoLink*>(
                           ImFactory::inject(k_imo_link, pDoc, get_node_id()) );


        // [<style>]
        analyse_optional_style(pLink);

        // <url>
        if (get_mandatory(k_url))
            set_url(pLink);

        // <inlineObject>+
        while( more_params_to_analyse() )
        {
            ImoInlineLevelObj* pItem = analyse_inline_object();
            if (pItem)
                pLink->add_item(pItem);

            move_to_next_param();
        }

        add_to_model(pLink);
    }

protected:

    void set_url(ImoLink* pLink)
    {
        // <url> = (url <string>)

        LdpElement* pValue = m_pParamToAnalyse->get_first_child();
        string url = pValue->get_value();
        pLink->set_url(url);
    }
};

//@--------------------------------------------------------------------------------------
//@ <list> = ("itemizedlist" | "orderedlist" [<style>] <listitem>* )
//@
class ListAnalyser : public ElementAnalyser
{
public:
    ListAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ELdpElement type = m_pAnalysedNode->get_type();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoList* pList = static_cast<ImoList*>(
                           ImFactory::inject(k_imo_list, pDoc, get_node_id()) );
        pList->set_list_type(type == k_itemizedlist ? ImoList::k_itemized
                                                    : ImoList::k_ordered);

        // [<style>]
        analyse_optional_style(pList);

        // <listitem>*
        while (analyse_optional(k_listitem, pList));
        error_if_more_elements();

        add_to_model(pList);
    }
};

//@--------------------------------------------------------------------------------------
//@ <listitem> = (listitem [<style>] {<inlineObject> | <blockObject>}* )
//@
class ListItemAnalyser : public ElementAnalyser
{
public:
    ListItemAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoListItem* pListItem = static_cast<ImoListItem*>(
                        ImFactory::inject(k_imo_listitem, pDoc, get_node_id()) );
        // [<style>]
        analyse_optional_style(pListItem);

        // {<inlineObject> | <blockObject>}*
        analyse_inline_or_block_objects(pListItem);

        add_to_model(pListItem);
    }
};

//@--------------------------------------------------------------------------------------
//@ lyric : (lyric [<lyricId>] <lyricText> [<style>][<placement>] <printOptions>* )
//@ lyricId : num.  Default 1
//@ lyricText : string+ [<hyphen>][<melisma>]
//@ hyphen : "-"
//@ melisma : (melisma)
//@ language : (lang string)
//@

class LyricAnalyser : public ElementAnalyser
{
public:
    LyricAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoNote* pNote = nullptr;
        if (m_pAnchor && m_pAnchor->is_note())
            pNote = static_cast<ImoNote*>(m_pAnchor);

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLyric* pImo = static_cast<ImoLyric*>(
                            ImFactory::inject(k_imo_lyric, pDoc, get_node_id()) );

        // [<lyricId>]
        int line = 1;
        if (get_optional(k_number))
            line = get_integer_value(1);
        pImo->set_number(line);

        // <syllable>+
        if (get_optional(k_string))
        {
            ImoLyricsTextInfo* pSyl = add_syllable(pImo, get_string_value());

            while (get_optional(k_string))
            {
                pSyl->set_elision_text(".");    //undertie U+203F
                //pSyl->set_elision_text("\xE2\x80\xBF");   //undertie U+203F in utf-8
                //pSyl->set_elision_text("0x203F");         //undertie U+203F
                //undertie is not supported in LiberationSerif font

                pSyl = add_syllable(pImo, get_string_value());
            }
        }
        else
        {
            error_msg("<lyric>: Missing syllable text. <lyric> ignored.");
            delete pImo;
            return;
        }

        // [<hyphen>] | [<placement>]
        // AWARE: if no hyphen and no melisma k_label can be placement
        bool fPlacement = false;
        if (get_optional(k_label))
        {
            string label = m_pParamToAnalyse->get_value();
            if (label == "-")
            {
                pImo->set_hyphenation(true);
            }
            else if (label == "above")
            {
                m_pAnalyser->set_lyrics_placement(line, k_placement_above);
                pImo->set_placement(k_placement_above);
                fPlacement = true;
            }
            else if (label == "below")
            {
                m_pAnalyser->set_lyrics_placement(line, k_placement_below);
                pImo->set_placement(k_placement_below);
                fPlacement = true;
            }
            else
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                    "<lyric>: Unknown parameter '" + label + "'. Ignored.");
            }
        }

        if (!fPlacement)
        {
            // [<melisma>]
            if (get_optional(k_melisma))
                pImo->set_melisma(true);

            // [<style>]
            analyse_optional_style(pImo);

            // [<placement>]
            if (get_optional(k_label))
            {
                int placement = get_placement(k_placement_below);
                m_pAnalyser->set_lyrics_placement(line, placement);
                pImo->set_placement(placement);
                fPlacement = true;
            }
        }

        if (!fPlacement)
            pImo->set_placement( m_pAnalyser->get_lyrics_placement(line) );

        // <printOptions>*
        analyse_scoreobj_options(pImo);

        error_if_more_elements();

        if (pNote)
        {
            m_pAnalyser->add_lyric(pNote, pImo);
            add_to_model(pImo);
        }
        else if (!m_libraryScope.is_unit_test())
        {
            error_msg("<lyric> is not attached to a note. It will be ignored.");
            delete pImo;
        }
        else
            m_pAnalysedNode->set_imo(pImo);
    }

protected:

    ImoLyricsTextInfo* add_syllable(ImoLyric* pImo, const string& text)
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLyricsTextInfo* pText = static_cast<ImoLyricsTextInfo*>(
                                    ImFactory::inject(k_imo_lyrics_text_info, pDoc) );
        pImo->add_text_item(pText);
        pText->set_syllable_text(text);
        return pText;
    }

};

//@--------------------------------------------------------------------------------------
//@ <metronome> = (metronome { <NoteType><TicksPerMinute> | <NoteType><NoteType> |
//@                            <TicksPerMinute> }
//@                          [parenthesis]<printOptions>* )
//@
//@ examples:
//@    (metronome q 80)                -->  quarter_note_sign = 80
//@    (metronome q q.)                -->  quarter_note_sign = dotted_quarter_note_sign
//@    (metronome 80)                  -->  m.m. = 80
//@    (metronome q 80 parenthesis)    -->  (quarter_note_sign = 80)

class MetronomeAnalyser : public ElementAnalyser
{
public:
    MetronomeAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMetronomeMark* pMtr = static_cast<ImoMetronomeMark*>(
                    ImFactory::inject(k_imo_metronome_mark, pDoc, get_node_id()) );

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
                report_msg(m_pAnalysedNode->get_line_number(),
                        "Error in metronome parameters. Replaced by '(metronome 60)'.");
                pMtr->set_ticks_per_minute(60);
                pMtr->set_mark_type(ImoMetronomeMark::k_value);
                add_to_model(pMtr);
                return;
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
            report_msg(m_pAnalysedNode->get_line_number(),
                    "Missing metronome parameters. Replaced by '(metronome 60)'.");
            pMtr->set_ticks_per_minute(60);
            pMtr->set_mark_type(ImoMetronomeMark::k_value);
            add_to_model(pMtr);
            return;
        }

        // [parenthesis]
        if (get_optional(k_label))
        {
            if (m_pParamToAnalyse->get_value() == "parenthesis")
                pMtr->set_parenthesis(true);
            else
                error_invalid_param();
        }

        // <printOptions>*
        analyse_scoreobj_options(pMtr);

        error_if_more_elements();

        add_to_model(pMtr);
    }
};

//@--------------------------------------------------------------------------------------
//@ <musicData> = (musicData [{<note>|<rest>|<barline>|<chord>|<clef>|<direction>|
//@                            <figuredBass>|<key>|<metronome>|<newSystem>|<spacer>|
//@                            <time>|<goFwd>|<goBack>|<graphic>|<line>|<text>}*] )
//@
//@ <graphic>, <line> and <text> elements are accepted for compatibility with 1.5.
//@ From 1.6 these elements will no longer be possible. They must go attached to an
//@ spacer or other staffobj
//@
//@ <metronome> element is accepted for backwards compatibility with 2.0. But in
//@ future <metronome> element will not be possible here. It must go attached to a
//@ <direction> element.


class MusicDataAnalyser : public ElementAnalyser
{
public:
    MusicDataAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                    ImFactory::inject(k_imo_music_data, pDoc, get_node_id()) );

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_params_to_analyse())
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
                   || analyse_optional(k_direction, pMD)
#if LOMSE_COMPATIBILITY_LDP_1_5
                   || analyse_optional(k_graphic, pMD)
                   || analyse_optional(k_line, pMD)
                   || analyse_optional(k_text, pMD)
#endif
                  ))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pMD);
        add_last_measure_info_if_required();
    }

protected:

    void add_last_measure_info_if_required()
    {
        ImoInstrument* pInstr = m_pAnalyser->get_current_instrument();
        if (pInstr != nullptr)  //in Unit Tests there could be no instrument
        {
            TypeMeasureInfo* pInfo = m_pAnalyser->get_measure_info();
            pInstr->set_last_measure_info(pInfo);
            m_pAnalyser->set_measure_info(nullptr);
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ ImoNote, ImoRest StaffObj
//@ <note> = ({n | na} <pitch><duration> <abbreviatedElements>* <noteOptions>*
//@                    <noteRestOptions>* <staffObjOptions>* <attachments>* )
//@ <rest> = (r <duration> <abbreviatedElements>* <noteRestOptions>*
//@             <staffObjOptions>* <attachments>* )
//@ <abbreviatedElements> = tie (l), beam (g), staffNum (p), tuplet' (t) and voice (v)
//@ <noteOptions> = { <tie> | <stem> | <slur> | <lyric> }
//@ <noteRestOptions> = { <beam> | <tuplet> | <voice> }
//@ <staffObjOptions> = { <staffNum> | <printOptions> }
//@ <printOptions> = { [<visible>] [<location>] [<color>] }
//@ <attachments> = { <text> | <textbox> | <line> | <fermata> | <dynamics> }
//@ <pitch> = label
//@ <duration> = label
//@ <stem> = (stem { up | down })
//@ <voice> = (voice num)
//@ <staffNum> = (p num)

class NoteRestAnalyser : public ElementAnalyser
{
protected:
    ImoTieDto* m_pTieDto;
    ImoTupletDto* m_pTupletInfo;
    ImoBeamDto* m_pBeamInfo;
    ImoSlurDto* m_pSlurDto;
    ImoFermata* m_pFermata;
    ImoTimeModificationDto* m_pTimeModifDto;
    std::string m_srcOldBeam;
    std::string m_srcOldTuplet;

public:
    NoteRestAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pTieDto(nullptr)
        , m_pTupletInfo(nullptr)
        , m_pBeamInfo(nullptr)
        , m_pSlurDto(nullptr)
        , m_pFermata(nullptr)
        , m_pTimeModifDto(nullptr)
        , m_srcOldBeam("")
        , m_srcOldTuplet("")
    {
    }

    void do_analysis()
    {
        bool fIsRest = m_pAnalysedNode->is_type(k_rest);
        bool fInChord = !fIsRest && m_pAnalysedNode->is_type(k_na);

        // create object note or rest
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoNoteRest* pNR = nullptr;
        ImoNote* pNote = nullptr;
        ImoRest* pRest = nullptr;
        if (fIsRest)
        {
            pRest = static_cast<ImoRest*>(
                          ImFactory::inject(k_imo_rest, pDoc, get_node_id()) );
            pNR = pRest;
        }
        else
        {
            pNote = static_cast<ImoNote*>(
                          ImFactory::inject(k_imo_note, pDoc, get_node_id()) );
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

        //abbreviatedElements
        //after duration we can find abbreviated items (l, g, p, v, t) in any order
        bool fStartOldTie = false;
        bool fAddOldBeam = false;
        bool fAddOldTuplet = false;
        while(get_optional(k_label))
        {
            char firstChar = (m_pParamToAnalyse->get_value())[0];
            if (!fIsRest && firstChar == 'l')
                fStartOldTie = true;
            else if (firstChar == 'g')
            {
                fAddOldBeam = true;
                m_srcOldBeam = m_pParamToAnalyse->get_value();
            }
            else if (firstChar == 't')
            {
                fAddOldTuplet = true;
                m_srcOldTuplet = m_pParamToAnalyse->get_value();
            }
            else if (firstChar == 'v')
                get_voice();
            else if (firstChar == 'p')
                get_num_staff();
            else
                error_invalid_param();
        }
        pNR->set_staff( m_pAnalyser->get_current_staff() );
        pNR->set_voice( m_pAnalyser->get_current_voice() );


        if (!fIsRest)
        {
            // [<noteOptions>*] = [{ <tie> | <stem> | <slur> | <lyric> }*]
            while (more_params_to_analyse())
            {
                if (get_optional(k_tie))
                    m_pTieDto = static_cast<ImoTieDto*>( proceed(k_tie, nullptr) );
                else if (get_optional(k_stem))
                    set_stem(pNote);
                else if (get_optional(k_slur))
                    m_pSlurDto = static_cast<ImoSlurDto*>( proceed(k_slur, nullptr) );
                else if (get_optional(k_lyric))
                    proceed(k_lyric, pNR);
                else
                    break;
            }
        }

        // [<noteRestOptions>*]
        analyse_note_rest_options(pNR);

        // [<staffObjOptions>*]
        analyse_staffobjs_options(pNR);

        add_to_model(pNR);

        // [<attachments>*]
        analyse_noterest_attachments(pNR);

        // add fermata
        if (m_pFermata)
            add_attachment(pNR, m_pFermata);

        //tie
        if (fStartOldTie)
            m_pAnalyser->start_old_tie(pNote, m_pParamToAnalyse);
        else if (!fIsRest)
            m_pAnalyser->create_tie_if_old_syntax_tie_pending(pNote);

        add_tie_info(pNote);

        //tuplet
        if (fAddOldTuplet)
            set_old_tuplet(pNR);
        else
            add_tuplet_info(pNR);

        //time modification
        if (m_pTimeModifDto != nullptr)
        {
            pNR->set_time_modification( m_pTimeModifDto->get_top_number(),
                                        m_pTimeModifDto->get_bottom_number() );
            delete m_pTimeModifDto;
        }


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

        //TODO: check if new note the same duration and voice than base note
      //  if (fInChord && m_pLastNote
      //      && !IsEqualTime(pStartOfChordNote->GetDuration(), rDuration) )
      //  {
      //      report_msg("Error: note in chord has different duration than base note. Duration changed.");
		    //rDuration = pStartOfChordNote->GetDuration();
      //      nNoteType = pStartOfChordNote->GetNoteType();
      //      nDots = pStartOfChordNote->GetNumDots();
      //  }

            ImoTreeAlgoritms::add_note_to_chord(pStartOfChordNote, pNote, pDoc);
        }

        //save this note as last note
        if (!fIsRest)
            m_pAnalyser->save_last_note(pNote);
    }

protected:

    void analyse_note_rest_options(ImoNoteRest* pNR)
    {
        // { <beam> | <tuplet> | <timeModification> | <voice> }

        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (type == k_tuplet)
            {
                m_pTupletInfo = static_cast<ImoTupletDto*>(
                        m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr) );
            }
            else if (type == k_time_modification)
            {
                m_pTimeModifDto = static_cast<ImoTimeModificationDto*>(
                        m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr) );
            }
            else if (type == k_beam)
            {
                m_pBeamInfo = static_cast<ImoBeamDto*>(
                        m_pAnalyser->analyse_node(m_pParamToAnalyse, nullptr) );
            }
            else if (type == k_voice)
            {
                set_voice_element(pNR);
            }
            else
                break;

            move_to_next_param();
        }
    }

    void set_notated_pitch(ImoNote* pNote)
    {
        string pitch = m_pParamToAnalyse->get_value();
        int step, octave;
        EAccidentals accidentals = k_no_accidentals;
        if (pitch == "*")
            pNote->set_notated_pitch(k_no_pitch, 4, k_no_accidentals);
        else
        {
            if (LdpAnalyser::ldp_pitch_to_components(pitch, &step, &octave, &accidentals))
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown note pitch '" + pitch + "'. Replaced by 'c4'.");
                pNote->set_notated_pitch(k_step_C, 4, k_no_accidentals);
            }
            else
            {
                pNote->set_notated_pitch(step, octave, accidentals);
                if (accidentals != k_no_accidentals)
                    pNote->force_to_display_accidentals();
            }
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid parameter '" + m_pParamToAnalyse->get_value()
                + "'. Ignored.");
        }
    }

    void set_stem(ImoNote* pNote)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string value = get_string_value();
        if (value == "up")
            pNote->set_stem_direction(k_stem_up);
        else if (value == "down")
            pNote->set_stem_direction(k_stem_down);
        else
        {
            pNote->set_stem_direction(k_stem_default);
            report_msg(m_pParamToAnalyse->get_line_number(),
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
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );
        m_pAnalyser->add_old_beam(pInfo);
    }

    void error_note_longer_than_eighth()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
            "Requesting beaming a note longer than eighth. Beam ignored.");
    }

    void error_beam_already_open()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
            "Requesting to start a beam (g+) but there is already an open beam. Beam ignored.");
    }

    void error_no_beam_open()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
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
            pInfo->set_line_number( m_pAnalysedNode->get_line_number() );
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid parameter '" + m_pParamToAnalyse->get_value()
                + "'. Ignored.");
        }
        else
        {
            if (m_pAnalyser->is_tuplet_open())
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                    "Requesting to start a tuplet but there is already an open tuplet. Tuplet ignored.");
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
        pInfo->set_line_number( m_pParamToAnalyse->get_line_number() );
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
        pInfo->set_tuplet_type(ImoTupletDto::k_start);
        pInfo->set_note_rest(pNR);
        pInfo->set_actual_number(actual);
        pInfo->set_normal_number(normal);
        pInfo->set_line_number( m_pParamToAnalyse->get_line_number() );
        m_pAnalyser->add_relation_info(pInfo);
    }

    void set_voice_element(ImoNoteRest* pNR)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int voice = get_integer_value( m_pAnalyser->get_current_voice() );
        m_pAnalyser->set_current_voice(voice);
        pNR->set_voice(voice);
    }

    void set_staff_num_element(ImoNoteRest* pNR)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
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
            if (m_pTupletInfo->is_start_of_tuplet())
                m_pAnalyser->add_to_open_tuplets(pNR);

            m_pTupletInfo->set_note_rest(pNR);
            bool fEndOfTuplet = m_pTupletInfo->is_end_of_tuplet();
            m_pAnalyser->add_relation_info(m_pTupletInfo);

            if (fEndOfTuplet)
                m_pAnalyser->add_to_open_tuplets(pNR);
        }
        else
            m_pAnalyser->add_to_open_tuplets(pNR);
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

class OptAnalyser : public ElementAnalyser
{
public:
    OptAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // <name> (label)
        string name;
        if (get_mandatory(k_label))
            name = m_pParamToAnalyse->get_value();

        // <value> { number | label | string }
        if (get_optional(k_label) || get_optional(k_number) || get_optional(k_string))
        {
            bool ok = set_option(name);
            if (ok)
                error_if_more_elements();
            else
            {
                if ( is_bool_option(name) || is_number_long_option(name)
                     || is_number_float_option(name) || is_string_option(name) )
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Invalid value for option '" + name + "'. Option ignored.");
                else
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Invalid option '" + name + "'. Option ignored.");
            }
        }
        else
            error_msg("Missing value for option '" + name + "'. Option ignored.");
    }


    bool set_option(string& name)
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
                    pOpt->set_long_value(k_justify_barline_final);
                else
                    pOpt->set_long_value(k_justify_never);
            }
            else if (name == "StaffLines.StopAtFinalBarline")
            {
                pOpt->set_name("StaffLines.Truncate");
                pOpt->set_type(ImoOptionInfo::k_number_long);
                if (pOpt->get_bool_value())
                    pOpt->set_long_value(k_truncate_barline_final);
                else
                    pOpt->set_long_value(k_truncate_never);
            }
        }


        if (fOk)
            add_to_model(pOpt);
        else
            delete pOpt;

        return fOk;
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
            || (name == "Render.SpacingValue")
            || (name == "Score.JustifyLastSystem")
            || (name == "Staff.UpperLegerLines.Displacement")
            || (name == "StaffLines.Truncate")
            ;
    }

    bool is_number_float_option(const string& name)
    {
        return (name == "Render.SpacingFactor")
            || (name == "Render.SpacingFopt")
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
        string value = m_pParamToAnalyse->get_value();
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

class PageLayoutAnalyser : public ElementAnalyser
{
public:
    PageLayoutAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Invalid orientation. Expected 'portrait' or 'landscape'."
                    " 'portrait' assumed.");
            pInfo->set_portrait(true);
        }

        error_if_more_elements();

        add_to_model(pInfo);
    }

protected:

    bool set_orientation(ImoPageInfo* pInfo)
    {
        // return true if ok
        string type = m_pParamToAnalyse->get_value();
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

class PageMarginsAnalyser : public ElementAnalyser
{
public:
    PageMarginsAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
        else
            return;         //what is this for?
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

    }

};

//@--------------------------------------------------------------------------------------
//@ <pageSize> = (pageSize width height)        LUnits

class PageSizeAnalyser : public ElementAnalyser
{
public:
    PageSizeAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = static_cast<ImoPageInfo*>(m_pAnchor);
        else
            return;     //what is this for?
            //pDto = &dto;

        //width
        if (get_mandatory(k_number))
            pDto->set_page_width( get_float_value(21000.0f) );

        //height
        if (get_mandatory(k_number))
            pDto->set_page_height( get_float_value(29700.0f) );
    }
};

//@--------------------------------------------------------------------------------------
//@ <paragraph> = (para [<style>] <inlineObject>*)

class ParagraphAnalyser : public ElementAnalyser
{
public:
    ParagraphAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoParagraph* pPara = static_cast<ImoParagraph*>(
                            ImFactory::inject(k_imo_para, pDoc, get_node_id()) );
        // [<style>]
        analyse_optional_style(pPara);

        // <inlineObject>*
        analyse_inline_objects(pPara);

        add_to_model(pPara);
    }
};

//@--------------------------------------------------------------------------------------
// <param> = (param <name> [<value>])
// <name> = <label>
// <value> = <string>

class ParamAnalyser : public ElementAnalyser
{
public:
    ParamAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        string name;
        string value = "";

        // <name>
        if (get_optional(k_label))
            name = get_string_value();
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Missing name for element 'param' (should be a label). Element ignored."
                );
            return;
        }

        //<value>
        if (get_optional(k_string))
            value = get_string_value();

        error_if_more_elements();


        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoParamInfo* pParam = static_cast<ImoParamInfo*>(
                        ImFactory::inject(k_imo_param_info, pDoc, get_node_id()) );
        pParam->set_name(name);
        pParam->set_value(value);
        add_to_model(pParam);
    }
};

//@--------------------------------------------------------------------------------------
//@ <parts> = (parts <instrIds> <group>*)
//@ <instrIds> (instrIds id+)
//@ id = A label. Must begin with a letter ([A-Za-z]) and may be followed by any number
//@      of letters, digits ([0-9]), hyphens ("-"), underscores ("_"), colons (":"),
//@      and periods (".").
//@
class PartsAnalyser : public ElementAnalyser
{
public:
    PartsAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();

        // <instrIds>
        if (get_optional(k_instrIds))
        {
            int numParts = m_pParamToAnalyse->get_num_parameters();
            for (int i=1; i <= numParts; ++i)
                add_instrument(i, pScore);
            m_pAnalyser->require_instr_id();
        }
        else
        {
            error_msg("parts: at least one <partId> is required.");
            return;
        }

        // <group>*
        while (analyse_optional(k_group, pScore));

        error_if_more_elements();
    }

protected:

    void add_instrument(int nInstr, ImoScore* pScore)
    {
        string partId = m_pParamToAnalyse->get_parameter(nInstr)->get_value();
        if (pScore->get_instrument(partId) == nullptr)
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                        ImFactory::inject(k_imo_instrument, pDoc) );
            pScore->add_instrument(pInstr, partId);
        }
        else
        {
            error_msg("parts: duplicated <partId> will be ignored.");
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <point> = (tag (dx value)(dy value))

class PointAnalyser : public ElementAnalyser
{
public:
    PointAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoPointDto point;

        // <dx>
        if (get_mandatory(k_dx))
            point.set_x( get_location_param() );

        // <dy>
        if (get_mandatory(k_dy))
            point.set_y( get_location_param() );

        error_if_more_elements();

        add_to_model( LOMSE_NEW ImoPointDto(point) );
    }
};

//@--------------------------------------------------------------------------------------
//@ <settings> = (settings [<cursor>][<undoData>])

class SettingsAnalyser : public ElementAnalyser
{
public:
    SettingsAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // [<cursor>]
        analyse_optional(k_cursor, m_pAnchor);

        // [<undoData>]
        analyse_optional(k_undoData, m_pAnchor);

        error_if_more_elements();
    }
};

//@--------------------------------------------------------------------------------------
//@ <score> = (score <vers>[<language>][<style>][<undoData>][<creationMode>]
//@                  [<defineStyle>*][<title>*][<pageLayout>*][<systemLayout>*]
//@                  [<option>*][<parts>]<instrument>* )

class ScoreAnalyser : public ElementAnalyser
{
public:
    ScoreAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScore* pScore = static_cast<ImoScore*>(
                              ImFactory::inject(k_imo_score, pDoc, get_node_id()) );
        m_pAnalyser->score_analysis_begin(pScore);
        add_to_model(pScore);

        // <vers>
        if (get_mandatory(k_vers))
            set_version(pScore);

        set_defaults_for_this_score_version(pScore);

        // [<language>]
        analyse_optional(k_language);

        // [<style>]
        analyse_optional_style(pScore);

        // [<undoData>]
        //TODO: Not implemented in LenMus. Postponed until need is confirmed
        analyse_optional(k_undoData, pScore);

        // [<creationMode>]
        analyse_optional(k_creationMode, pScore);

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pScore));
        pScore->add_required_text_styles();

        // [<title>*]
        while (analyse_optional(k_title, pScore));

        // [<pageLayout>*]
        while (analyse_optional(k_pageLayout, pScore));

        // [<systemLayout>*]
        while (analyse_optional(k_systemLayout, pScore));

        // [<cursor>]
        // Obsolete since 1.6, as cursor is now a document attribute
        if (get_optional(k_cursor))
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "'cursor' in score is obsolete. Now must be in 'lenmusdoc' element. Ignored.");

        // [<option>*]
        while (analyse_optional(k_opt, pScore));

        // [<parts>]
        analyse_optional(k_parts, pScore);

        // <instrument>*
        if (!more_params_to_analyse())
            error_missing_element(k_instrument);
        else
        {
            while (more_params_to_analyse())
            {
                if (! (analyse_optional(k_instrument, pScore)))
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
        }

        error_if_more_elements();

        m_pAnalyser->score_analysis_end();
    }

protected:

    void set_version(ImoScore* pScore)
    {
        string vers = m_pParamToAnalyse->get_parameter(1)->get_value();

        //check that version is valid
        if (vers == "1.5" || vers == "1.6" || vers == "1.7" || vers == "2.0")
        {
            //1.5 -
            //1.6 -
            //1.7 - transitional to 2.0. Was not published
            //2.0
        }
        else if (vers == "2.1")
        {
            //2.1 - sep/2006
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid score version '" + vers + "'. Version 1.6 assumed.");
            vers = "1.6";
        }
        int numvers = m_pAnalyser->set_score_version(vers);
        pScore->set_version(numvers);
    }

    void set_defaults_for_this_score_version(ImoScore* pScore)
    {
        //set default options for each score version
        string version = pScore->get_version_string();
        if (version == "2.1")
        {
            ImoOptionInfo* pOpt = pScore->get_option("Render.SpacingFopt");
            pOpt->set_float_value(1.0f);

            pOpt = pScore->get_option("Render.SpacingOptions");
            pOpt->set_long_value(k_render_opt_breaker_optimal | k_render_opt_dmin_global);
        }
        else
        {
            //vers. 1.5, 1.6, 1.7, 2.0. Default values are ok
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <scorePlayer> = (scorePlayer [<style>] <opt>*)
//@ <opt> = { <mm> | <playLabel> | <stopLabel> }
//@ <mm> = (mm <integer>)
//@ <playLabel> = (playLabel <string>)
//@ <stopLabel> = (stopLabel <string>)
//@
class ScorePlayerAnalyser : public ElementAnalyser
{
public:
    ScorePlayerAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScorePlayer* pSP = static_cast<ImoScorePlayer*>(
                    ImFactory::inject(k_imo_score_player, pDoc, get_node_id()) );
        ScorePlayerCtrl* pPlayer = LOMSE_NEW ScorePlayerCtrl(m_libraryScope, pSP, pDoc);
        pSP->attach_player(pPlayer);
        pSP->attach_score( m_pAnalyser->get_last_analysed_score() );

        // [<style>]
        analyse_optional_style(pSP);

        // <opt>*
        while (more_params_to_analyse())
        {
            if (get_optional(k_mm))
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                pSP->set_metronome_mm( get_integer_value(60) );
            }
            else if (get_optional(k_playLabel))
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                pSP->set_play_label( get_string_value() );
            }
            else if (get_optional(k_stopLabel))
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                pSP->set_stop_label( get_string_value() );
            }
            else
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pSP);
    }
};

//@--------------------------------------------------------------------------------------
//@ <size> = (size <width><height>)
//@ <width> = (width number)        value in LUnits
//@ <height> = (height number)      value in LUnits
//@     i.e.; (size (width 160)(height 100.7))

class SizeAnalyser : public ElementAnalyser
{
public:
    SizeAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSizeDto size;

        // <width>
        if (get_mandatory(k_width))
            size.set_width( get_width_param() );

        // <height>
        if (get_mandatory(k_height))
            size.set_height( get_height_param() );

        error_if_more_elements();

        add_to_model( LOMSE_NEW ImoSizeDto(size) );
    }
};


//@--------------------------------------------------------------------------------------
//@ <slur> = (slur num <slurType>[<bezier>][color] )   ;num = slur number. integer
//@ <slurType> = { start | continue | stop }
//@
//@ Example:
//@     (slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )
//@

class SlurAnalyser : public ElementAnalyser
{
public:
    SlurAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoSlurDto* pInfo = LOMSE_NEW ImoSlurDto();
        pInfo->set_id( get_node_id() );
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );

        // num
        if (get_mandatory(k_number))
            pInfo->set_slur_number( get_integer_value(0) );

        // <slurType> (label)
        if (!get_mandatory(k_label) || !set_slur_type(pInfo))
        {
            error_msg("Missing or invalid slur type. Slur ignored.");
            delete pInfo;
            return;
        }

        // [<bezier>]
        analyse_optional(k_bezier, pInfo);

        // [<color>]
        if (get_optional(k_color))
            pInfo->set_color( get_color_param() );

        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    bool set_slur_type(ImoSlurDto* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
        if (value == "start")
            pInfo->set_start(true);
        else if (value == "stop")
            pInfo->set_start(false);
        else if (value == "continue")
            pInfo->set_start(false);        //TODO: Continue?
        else
            return false;   //error
        return true;    //ok
    }
};

//@--------------------------------------------------------------------------------------
//@ ImoDirection StaffObj
//@ <spacer> = (spacer <width> <staffobjOptions>* <soAttachments>*)
//@
//@ width in Tenths

class SpacerAnalyser : public ElementAnalyser
{
public:
    SpacerAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDirection* pSpacer = static_cast<ImoDirection*>(
                    ImFactory::inject(k_imo_direction, pDoc, get_node_id()) );

        // <width>
        if (get_optional(k_number))
        {
            pSpacer->set_width( get_float_value() );
        }
        else
        {
            error_msg("Missing width for spacer. Spacer ignored.");
            delete pSpacer;
            return;
        }

        // <staffobjOptions>*
        analyse_staffobjs_options(pSpacer);

        // <soAttachments>*
        analyse_staffobj_attachments(pSpacer);

        add_to_model(pSpacer);
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

class StaffAnalyser : public ElementAnalyser
{
public:
    StaffAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                    ImFactory::inject(k_imo_staff_info, pDoc, get_node_id()) );

        // num_instr
        if (!get_optional(k_number) || !set_staff_number(pInfo))
        {
            error_msg("Missing or invalid staff number. Staff info ignored.");
            delete pInfo;
            return;
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
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pInfo->set_line_spacing( get_float_value(180.0f));
        }

        //[<staffDistance>]
        if (get_optional(k_staffDistance))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pInfo->set_staff_margin( get_float_value(1000.0f));
        }

        //[<lineThickness>]
        if (get_optional(k_lineThickness))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pInfo->set_line_thickness( get_float_value(15.0f));
        }

        error_if_more_elements();

        add_to_model(pInfo);
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
        const std::string& value = m_pParamToAnalyse->get_parameter(1)->get_value();
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
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);

        int value = get_integer_value(0);
        if (value < 1)
            error_msg("Invalid staff. Num lines must be greater than zero. Five assumed.");
        else
            pInfo->set_num_lines(value);
    }


};

//@--------------------------------------------------------------------------------------
//@ <styles> = (styles <defineStyle>*)

class StylesAnalyser : public ElementAnalyser
{
public:
    StylesAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStyles* pStyles = static_cast<ImoStyles*>(
                         ImFactory::inject(k_imo_styles, pDoc, get_node_id()) );

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pStyles));

        error_if_more_elements();

        add_to_model(pStyles);
    }

};

//@--------------------------------------------------------------------------------------
//@ <systemLayout> = (systemLayout {first | other} <systemMargins>)

class SystemLayoutAnalyser : public ElementAnalyser
{
public:
    SystemLayoutAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemInfo* pInfo = static_cast<ImoSystemInfo*>(
                        ImFactory::inject(k_imo_system_info, pDoc, get_node_id()) );

        // {first | other} <label>
        if (get_mandatory(k_label))
        {
            string type = m_pParamToAnalyse->get_value();
            if (type == "first")
                pInfo->set_first(true);
            else if (type == "other")
                pInfo->set_first(false);
            else
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Expected 'first' or 'other' value but found '" + type
                        + "'. 'first' assumed.");
                pInfo->set_first(true);
            }
        }

        // <systemMargins>
        analyse_mandatory(k_systemMargins, pInfo);

        error_if_more_elements();

        add_to_model(pInfo);
    }

};

//@--------------------------------------------------------------------------------------
//@ <systemMargins> = (systemMargins <leftMargin><rightMargin><systemDistance>
//@                                  <topSystemDistance>)
//@ <leftMargin>, <rightMargin>, <systemDistance>, <topSystemDistance> = number (Tenths)

class SystemMarginsAnalyser : public ElementAnalyser
{
public:
    SystemMarginsAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoSystemInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_system_info())
            pDto = static_cast<ImoSystemInfo*>(m_pAnchor);
        else
            return;     //what is this for?

        if (get_mandatory(k_number))
            pDto->set_left_margin(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_right_margin(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_system_distance(get_float_value());

        if (get_mandatory(k_number))
            pDto->set_top_system_distance(get_float_value());

        error_if_more_elements();
    }

};

//@--------------------------------------------------------------------------------------
//@ (table [<style>] [<tableColumn>*] [<tableHead>] <tableBody> )
//@

class TableAnalyser : public ElementAnalyser
{
public:
    TableAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableBody> = (tableBody <tableRow>* )
//@

class TableBodyAnalyser : public ElementAnalyser
{
public:
    TableBodyAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableBody* pBody = static_cast<ImoTableBody*>(
                        ImFactory::inject(k_imo_table_body, pDoc, get_node_id()) );

        // <tableRow>*
        while( more_params_to_analyse() )
        {
            analyse_mandatory(k_table_row, pBody);
        }

        add_to_model(pBody);
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableCell> = (tableCell [<style>] [<rowspan>] [<colspan>]
//@                          {<inlineObject> | <blockObject>}* )
//@ <rowspan> = (rowspan <num>)
//@ <colspan> = (colspan <num>)
//@

class TableCellAnalyser : public ElementAnalyser
{
public:
    TableCellAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableCell* pImo = static_cast<ImoTableCell*>(
                    ImFactory::inject(k_imo_table_cell, pDoc, get_node_id()) );
        //[<style>]
        analyse_optional_style(pImo);

        //[<rowspan>]
        if (get_optional(k_rowspan))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pImo->set_rowspan( get_integer_value(1) );
        }

        //[<colspan>]
        if (get_optional(k_colspan))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pImo->set_colspan( get_integer_value(1) );
        }

        // {<inlineObject> | <blockObject>}*
        analyse_inline_or_block_objects(pImo);

        error_if_more_elements();

        add_to_model(pImo);
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableColumn> = (tableColumn <style>)
//@

class TableColumnAnalyser : public ElementAnalyser
{
public:
    TableColumnAnalyser(LdpAnalyser* pAnalyser, ostream& reporter,
                        LibraryScope& libraryScope, ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // <style>
        if (get_optional(k_style) && m_pAnchor->is_table())
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            ImoStyle* pStyle = get_doc_text_style( get_string_value() );
            ImoTable* pTable = static_cast<ImoTable*>(m_pAnchor);
            pTable->add_column_style(pStyle);
        }

        error_if_more_elements();
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableHead> = (tableHead <tableRow>* )
//@

class TableHeadAnalyser : public ElementAnalyser
{
public:
    TableHeadAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableHead* pHead = static_cast<ImoTableHead*>(
                        ImFactory::inject(k_imo_table_head, pDoc, get_node_id()) );

        // <tableRow>*
        while( more_params_to_analyse() )
        {
            analyse_mandatory(k_table_row, pHead);
        }

        add_to_model(pHead);
    }
};

//@--------------------------------------------------------------------------------------
//@ <tableRow> = (tableRow [<style>] <tableCell>* )
//@

class TableRowAnalyser : public ElementAnalyser
{
public:
    TableRowAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTableRow* pRow = static_cast<ImoTableRow*>(
                        ImFactory::inject(k_imo_table_row, pDoc, get_node_id()) );

        // [<style>]
        analyse_optional_style(pRow);

        // <tableCell>*
        while( more_params_to_analyse() )
        {
            analyse_mandatory(k_table_cell, pRow);
        }

        add_to_model(pRow);
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

class TextBoxAnalyser : public ElementAnalyser
{
public:
    TextBoxAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTextBlockInfo box;

        // <location>
        if (get_mandatory(k_dx))
            box.set_position_x( get_location_param() );
        if (get_mandatory(k_dy))
            box.set_position_y( get_location_param() );

        // <size>
        if (get_mandatory(k_size))
            box.set_size( get_size_param() );

        // [<bgColor>])
        if (get_optional(k_color))
            box.set_bg_color( get_color_param() );

        // [<border>]
        if (get_optional(k_border))
            set_box_border(box);

        ImoTextBox* pTB = ImFactory::inject_text_box(pDoc, box, get_node_id());

        // <text>
        if (get_mandatory(k_text))
            set_box_text(pTB);

        // [<anchorLine>]
        if (get_optional(k_anchorLine))
            set_anchor_line(pTB);

        error_if_more_elements();

        add_to_model(pTB);
    }

protected:

    void set_box_border(ImoTextBlockInfo& box)
    {
        ImoObj* pImo = proceed(k_border, nullptr);
        if (pImo)
        {
            if (pImo->is_border_dto())
                box.set_border( static_cast<ImoBorderDto*>(pImo) );
            delete pImo;
        }
    }

    void set_box_text(ImoTextBox* pTB)
    {
        ImoObj* pImo = proceed(k_text, nullptr);
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
        ImoObj* pImo = proceed(k_anchorLine, nullptr);
        if (pImo)
        {
            if (pImo->is_line_style())
                pTB->set_anchor_line( static_cast<ImoLineStyle*>(pImo) );
            delete pImo;
        }
    }

};

//@--------------------------------------------------------------------------------------
//@ <textItem> = (txt [<style>] [<location>] string)
//@ <style> = (style <name>)
//@     if no style is specified default style is assigned.
//@
class TextItemAnalyser : public ElementAnalyser
{
public:
    TextItemAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
    {
    }

    void do_analysis()
    {
        string styleName = "Default style";

        // [<style>]
        ImoStyle* pStyle = nullptr;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }

        //// [<location>]
        //while (more_params_to_analyse())
        //{
        //    if (get_optional(k_dx))
        //        pText->set_user_location_x( get_location_param() );
        //    else if (get_optional(k_dy))
        //        pText->set_user_location_y( get_location_param() );
        //    else if (get_optional(k_string)
        //        break;
        //    else
            //{
            //    error_invalid_param();
            //    move_to_next_param();
            //}
        //}

        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoTextItem* pText = static_cast<ImoTextItem*>(
                        ImFactory::inject(k_imo_text_item, pDoc, get_node_id()) );
            pText->set_text( get_string_value() );
            pText->set_style(pStyle);

            error_if_more_elements();

            add_to_model(pText);
        }
    }

protected:
    //void set_default_style(ImoScoreText* pText)
    //{
    //    ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
    //    if (pScore)
    //        pText->set_style( pScore->get_style_or_default(m_styleName) );
    //}

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
class TextStringAnalyser : public ElementAnalyser
{
protected:
    string m_styleName;

public:
    TextStringAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor, const string& styleName="Default style")
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_styleName(styleName)
    {
    }

    void do_analysis()
    {
        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoScoreText* pText = static_cast<ImoScoreText*>(
                        ImFactory::inject(k_imo_score_text, pDoc, get_node_id()));
            pText->set_text(get_string_value());

            // [<style>]
            ImoStyle* pStyle = nullptr;
            if (get_optional(k_style))
                pStyle = get_text_style_param(m_styleName);
            else
            {
                ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
                if (pScore)     //in unit tests the score might not exist
                    pStyle = pScore->get_default_style();
            }
            pText->set_style(pStyle);

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pText->set_user_location_x( get_location_param() );
                else if (get_optional(k_dy))
                    pText->set_user_location_y( get_location_param() );
                else
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
            error_if_more_elements();

            add_to_model(pText);
        }
    }

};

class InstrNameAnalyser : public TextStringAnalyser
{
public:
    InstrNameAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : TextStringAnalyser(pAnalyser, reporter, libraryScope, pAnchor,
                             "Instrument names") {}
};


//@--------------------------------------------------------------------------------------
//@ <tie> = (tie num <tieType>[<bezier>][color] )   ;num = tie number. integer
//@ <tieType> = { start | stop }

class TieAnalyser : public ElementAnalyser
{
public:
    TieAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //AWARE: ImoTieDto will be discarded. So no ID will be assigned to avoid
        //problems with undo/redo
        ImoTieDto* pInfo = LOMSE_NEW ImoTieDto();
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );

        // num
        if (get_mandatory(k_number))
            pInfo->set_tie_number( get_integer_value(0) );

        // <tieType> (label)
        if (!get_mandatory(k_label) || !set_tie_type(pInfo))
        {
            error_msg("Missing or invalid tie type. Tie ignored.");
            delete pInfo;
            return;
        }

        // [<bezier>]
        analyse_optional(k_bezier, pInfo);

        // [<color>]
        if (get_optional(k_color))
            pInfo->set_color( get_color_param() );

        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    bool set_tie_type(ImoTieDto* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
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
//@ <timeModification> = (tm <numerator> <denominator>)
//@ <numerator> = integer number
//@ <denominator> = integer number

class TimeModificationAnalyser : public ElementAnalyser
{
public:
    TimeModificationAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeModificationDto* pTime = static_cast<ImoTimeModificationDto*>(
                    ImFactory::inject(k_imo_time_modification_dto, pDoc, get_node_id()) );

        // <numerator> (num)
        if (get_mandatory(k_number))
        {
            pTime->set_top_number( get_integer_value(1) );

            // <denominator> (num)
            if (get_mandatory(k_number))
            {
                pTime->set_bottom_number( get_integer_value(1) );
                error_if_more_elements();
                add_to_model(pTime);
                return;
            }
        }
        delete pTime;
    }

};

//@--------------------------------------------------------------------------------------
//@ <timeSignature> = (time [<type>] { (<top><bottom>)+ | "senza-misura" }
//@                    [<visible>] [<location>] <soAttachments>*)
//@
//@ <type> = {"normal" | "common" | "cut" | "single-number"}  default: "normal"
//@ <top> = <num>
//@ <bottom> = <num>
//@
//@ Examples:
//@     (time 2 4)
//@     (time 3 8)
//@     (time 2 4 3 8) = 2/4 + 3/8
//@     (time 3 8 2 8) = 3+2/8
//@     (time common)
//@     (time cut)
//@     (time single-number 3)
//@     (time senza-misura)
//@
//@ TODO: CURRENTLY ONLY IMPLEMENTED:
//@     (time [normal] <top> <bottom> [<visible>] [<location>])
//@     (time common [<visible>] [<location>])
//@     (time cut [<visible>] [<location>])


class TimeSignatureAnalyser : public ElementAnalyser
{
public:
    TimeSignatureAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>(
                    ImFactory::inject(k_imo_time_signature, pDoc, get_node_id()) );

        // [<type>]
        if (get_optional(k_label))
            set_type(pTime);

        // <top><bottom>
        if (pTime->get_type() == ImoTimeSignature::k_normal)
        {
            // <top> (num)
            if (get_mandatory(k_number))
                pTime->set_top_number( get_integer_value(2) );

            // <bottom> (num)
            if (get_mandatory(k_number))
                pTime->set_bottom_number( get_integer_value(4) );
        }
        else if (pTime->get_type() == ImoTimeSignature::k_single_number)
        {
            // <top> (num)
            if (get_mandatory(k_number))
                pTime->set_top_number( get_integer_value(2) );
            pTime->set_bottom_number(0);
        }

        // [<visible>][<location>]
        analyse_staffobjs_options(pTime);

        // <soAttachments>*
        analyse_staffobj_attachments(pTime);

        add_to_model(pTime);
    }

protected:

    void set_type(ImoTimeSignature* pTime)
    {
        const std::string& type = m_pParamToAnalyse->get_value();
        if (type == "normal")
            return;
        else if (type == "common")
        {
            pTime->set_type(ImoTimeSignature::k_common);
            pTime->set_top_number(4);
            pTime->set_bottom_number(4);
            return;
        }
        else if (type == "cut")
        {
            pTime->set_type(ImoTimeSignature::k_cut);
            pTime->set_top_number(2);
            pTime->set_bottom_number(2);
            return;
        }
        else if (type == "single-number")
        {
            pTime->set_type(ImoTimeSignature::k_single_number);
            return;
        }
        else
            error_msg("Time signature: invalid type '" + type + "'. 'normal' assumed.");

        return;
    }

};

//@--------------------------------------------------------------------------------------
//@ <title> = (title [<h-align>] string [<style>][<location>])
//@ <h-align> = label: {left | center | right }
//@ <style> = (style name)
//@         name = string.  Must be a style name defined with defineStyle
//@
//@ IMPORTANT: <h-align> is obsolete and not used. It is maintained in the analyser
//@            until I check all tests scores
//@ Note:
//@     <h-align> overrides <style> (but doesn't modify it)
//@
//@ Examples:
//@     (title center "Prelude" (style "Title"))
//@     (title center "Op. 28, No. 20" (style "Subtitle"))
//@     (title right "F. Chopin" (style "Composer")(dy 30))

class TitleAnalyser : public ElementAnalyser
{
public:
    TitleAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // [<h-align>]
        int nAlignment = k_halign_left;
        if (get_optional(k_label))
            nAlignment = get_alignment_value(k_halign_center);

        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoScoreTitle* pTitle = static_cast<ImoScoreTitle*>(
                ImFactory::inject(k_imo_score_title, pDoc, get_node_id()) );
            pTitle->set_text( get_string_value() );
            pTitle->set_h_align(nAlignment);

            // [<style>]
            if (get_optional(k_style))
                pTitle->set_style( get_text_style_param() );

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pTitle->set_user_location_x( get_location_param() );
                else if (get_optional(k_dy))
                    pTitle->set_user_location_y( get_location_param() );
                else
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
            error_if_more_elements();

            add_to_model(pTitle);
        }
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
//@ <bracketType> = (bracketType { squaredBracket | curvedBracket })  [squaredBracket]
//@ <displayBracket> = (displayBracket { yes | no })  [yes]
//@ <displayNumber> = (displayNumber { none | actual | both })  [actual]

class TupletAnalyser : public ElementAnalyser
{
public:
    TupletAnalyser(LdpAnalyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoTupletDto* pInfo = LOMSE_NEW ImoTupletDto();
        pInfo->set_id( get_node_id() );
        set_default_values(pInfo);

        // [<tupletID>]     //optional. Only required for nested tuplets
        // <tupletID>   Mandatory since 2.1, for nested tuplets
        // if (m_pAnalyser->get_score_version() > 200)
        //{
        //    if (get_mandatory(k_number))
        //        set_tuplet_id(pInfo);
        //    else
        //    {
        //        error_msg("Missing or invalid tuplet number. Tuplet ignored.");
        //        delete pInfo;
        //        return;
        //    }
        //}
        //else
        {
            if (get_optional(k_number))
                set_tuplet_id(pInfo);
        }

        // { + | - }
        if (!get_mandatory(k_label) || !set_tuplet_type(pInfo))
        {
            error_msg("Missing or invalid tuplet type. Tuplet ignored.");
            delete pInfo;
            return;
        }

        if (pInfo->is_start_of_tuplet())
        {
            // <actualNotes>
            if (!get_mandatory(k_number) || !set_actual_notes(pInfo))
            {
                error_msg("Tuplet: missing or invalid actual notes number. Tuplet ignored.");
                delete pInfo;
                return;
            }

            // [<normalNotes>]
            if (get_optional(k_number))
                set_normal_notes(pInfo);
            if (pInfo->get_normal_number() == 0)
            {
                error_msg("Tuplet: Missing or invalid normal notes number. Tuplet ignored.");
                delete pInfo;
                return;
            }

            // [<tupletOptions>]
            analyse_tuplet_options(pInfo);
        }

        pInfo->set_only_graphical( m_pAnalyser->get_score_version() >= 107 );
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );
        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    void set_default_values(ImoTupletDto* pInfo)
    {
        pInfo->set_show_bracket( m_pAnalyser->get_current_show_tuplet_bracket() );
        pInfo->set_placement(k_placement_default);
        pInfo->set_line_number( m_pAnalysedNode->get_line_number() );
    }

    void set_tuplet_id(ImoTupletDto* pInfo)
    {
        //AWARE: tuplet number was not used before 2.0, when enabling nested tuplets
        if (m_pAnalyser->get_score_version() > 107)
            pInfo->set_tuplet_number( get_integer_value(0) );
        else
            pInfo->set_tuplet_number(0);
    }

    bool set_tuplet_type(ImoTupletDto* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
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
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            switch (type)
            {
                case k_label:
                {
                    const std::string& value = m_pParamToAnalyse->get_value();
                    if (value == "noBracket")
                        nShowBracket = k_yesno_no;
                    else
                        error_invalid_param();
                    break;
                }

                case k_displayBracket:
                {
                    m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                    nShowBracket = get_yes_no_value(k_yesno_default);
                    break;
                }

                case k_displayNumber:
                {
                    m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                    const std::string& value = m_pParamToAnalyse->get_value();
                    if (value == "none")
                        nShowNumber = ImoTuplet::k_number_none;
                    else if (value == "actual")
                        nShowNumber = ImoTuplet::k_number_actual;
                    else if (value == "both")
                        nShowNumber = ImoTuplet::k_number_both;
                    else
                    {
                        error_invalid_param();
                        nShowNumber = ImoTuplet::k_number_actual;
                    }
                    break;
                }

                default:
                    error_invalid_param();
            }

            move_to_next_param();
        }

        pInfo->set_show_bracket(nShowBracket);
        pInfo->set_show_number(nShowNumber);
        m_pAnalyser->set_current_show_tuplet_bracket(nShowBracket);
        m_pAnalyser->set_current_show_tuplet_number(nShowNumber);
    }

};



//=======================================================================================
// ElementAnalyser implementation
//=======================================================================================
void ElementAnalyser::analyse_node(LdpElement* pNode)
{
    m_pAnalysedNode = pNode;
    move_to_first_param();
    do_analysis();
}

//---------------------------------------------------------------------------------------
bool ElementAnalyser::error_missing_element(ELdpElement type)
{
    const string& parentName =
        m_pLdpFactory->get_name( m_pAnalysedNode->get_type() );
    const string& name = m_pLdpFactory->get_name(type);
    report_msg(m_pAnalysedNode->get_line_number(),
               parentName + ": missing mandatory element '" + name + "'.");
    return false;
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::report_msg(int numLine, const std::stringstream& msg)
{
    report_msg(numLine, msg.str());
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::report_msg(int numLine, const std::string& msg)
{
    m_reporter << "Line " << numLine << ". " << msg << endl;
}

//---------------------------------------------------------------------------------------
bool ElementAnalyser::get_mandatory(ELdpElement type)
{
    if (!more_params_to_analyse())
    {
        error_missing_element(type);
        return false;
    }

    m_pParamToAnalyse = get_param_to_analyse();
    if (m_pParamToAnalyse->get_type() != type)
    {
        error_missing_element(type);
        return false;
    }

    move_to_next_param();
    return true;
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::analyse_mandatory(ELdpElement type, ImoObj* pAnchor)
{
    if (get_mandatory(type))
        m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
}

//---------------------------------------------------------------------------------------
bool ElementAnalyser::get_optional(ELdpElement type)
{
    if (more_params_to_analyse())
    {
        m_pParamToAnalyse = get_param_to_analyse();
        if (m_pParamToAnalyse->get_type() == type)
        {
            move_to_next_param();
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------
bool ElementAnalyser::analyse_optional(ELdpElement type, ImoObj* pAnchor)
{
    if (get_optional(type))
    {
        m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::analyse_one_or_more(ELdpElement* pValid, int nValid)
{
    while(more_params_to_analyse())
    {
        m_pParamToAnalyse = get_param_to_analyse();

        ELdpElement type = m_pParamToAnalyse->get_type();
        if (contains(type, pValid, nValid))
        {
            move_to_next_param();
            m_pAnalyser->analyse_node(m_pParamToAnalyse);
        }
        else
        {
            string name = m_pLdpFactory->get_name(type);
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Element '" + name + "' unknown or not possible here. Ignored.");
        }
        move_to_next_param();
    }
}

//---------------------------------------------------------------------------------------
bool ElementAnalyser::contains(ELdpElement type, ELdpElement* pValid, int nValid)
{
    for (int i=0; i < nValid; i++, pValid++)
        if (*pValid == type) return true;
    return false;
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::error_invalid_param()
{
    ELdpElement type = m_pParamToAnalyse->get_type();
    string name = m_pLdpFactory->get_name(type);
    if (name == "label")
        name += ":" + m_pParamToAnalyse->get_value();
    report_msg(m_pParamToAnalyse->get_line_number(),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::error_msg(const string& msg)
{
    report_msg(m_pAnalysedNode->get_line_number(), msg);
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::error_if_more_elements()
{
    if (more_params_to_analyse())
    {
        ELdpElement type = m_pParamToAnalyse->get_type();
        string name = m_pLdpFactory->get_name(type);
        if (name == "label")
            name += ":" + m_pParamToAnalyse->get_value();
        report_msg(m_pAnalysedNode->get_line_number(),
                "Element '" + m_pAnalysedNode->get_name()
                + "': too many parameters. Extra parameters from '"
                + name + "' have been ignored.");
    }
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::analyse_staffobjs_options(ImoStaffObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <staffobjOptions> = { <staffNum> | <printOptions> }
    //@ <staffNum> = pn | (staffNum n)

    // [p1]
    if (get_optional(k_label))
    {
        char type = (m_pParamToAnalyse->get_value())[0];
        if (type == 'p')
            get_num_staff();
        else
            error_invalid_param();
    }

    // [<staffNum>]
    else if (get_optional(k_staffNum))
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        long nStaff = get_long_value(1L);
        m_pAnalyser->set_current_staff(--nStaff);
    }

    //set staff: either found value or inherited one
    pSO->set_staff( m_pAnalyser->get_current_staff() );

    analyse_scoreobj_options(pSO);
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::analyse_scoreobj_options(ImoScoreObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <printOptions> = { <visible> | <location> | <color> }
    //@ <visible> = (visible {yes | no})
    //@ <location> = { (dx num) | (dy num) }    ;num in Tenths
    //@ <color> = value                         ;value in #rrggbb hex format

    // [ { <visible> | <location> | <color> }* ]
    while( more_params_to_analyse() )
    {
        m_pParamToAnalyse = get_param_to_analyse();
        ELdpElement type = m_pParamToAnalyse->get_type();
        switch (type)
        {
            case k_visible:
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                pSO->set_visible( get_bool_value(true) );
                break;
            }
            case k_color:
            {
                pSO->set_color( get_color_param() );
                break;
            }
            case k_dx:
            {
                pSO->set_user_location_x( get_location_param() );
                break;
            }
            case k_dy:
            {
                pSO->set_user_location_y( get_location_param() );
                break;
            }
            default:
                return;
        }

        move_to_next_param();
    }
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::add_to_model(ImoObj* pImo)
{
    if (pImo->is_staffobj())
        create_measure_info_if_necessary();

    int ldpNodeType = m_pAnalysedNode->get_type();
    Linker linker( m_pAnalyser->get_document_being_analysed() );
    ImoObj* pObj = linker.add_child_to_model(m_pAnchor, pImo, ldpNodeType);
    m_pAnalysedNode->set_imo(pObj);
}

//---------------------------------------------------------------------------------------
void ElementAnalyser::create_measure_info_if_necessary()
{
    TypeMeasureInfo* pInfo = m_pAnalyser->get_measure_info();
    if (pInfo == nullptr)
    {
        pInfo = LOMSE_NEW TypeMeasureInfo();
        m_pAnalyser->set_measure_info(pInfo);
        pInfo->count = m_pAnalyser->increment_measures_counter();
    }
}


//=======================================================================================
// LdpAnalyser implementation
//=======================================================================================
LdpAnalyser::LdpAnalyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc)
    : Analyser()
    , m_reporter(reporter)
    , m_libraryScope(libraryScope)
    , m_pDoc(pDoc)
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
    , m_pTree()
    , m_fileLocator("")
    , m_curStaff(0)
    , m_curVoice(0)
    , m_nShowTupletBracket(k_yesno_default)
    , m_nShowTupletNumber(k_yesno_default)
    , m_pLastNote(nullptr)
    , m_pMeasureInfo(nullptr)
    , m_fInstrIdRequired(false)
    , m_measuresCounter(0)
    , m_pCurInstr(nullptr)
    , m_extraMarginSpace(0.0f)
{
}

//---------------------------------------------------------------------------------------
LdpAnalyser::~LdpAnalyser()
{
    delete_relation_builders();
    m_lyrics.clear();
    m_lyricIndex.clear();
    m_lyricsPlacement.clear();
}

//---------------------------------------------------------------------------------------
void LdpAnalyser::reset_defaults_for_instrument()
{
    m_curStaff = 0;
    m_curVoice = 1;
    m_pLastNote = nullptr;
    m_pMeasureInfo = nullptr;
    m_pCurInstr = nullptr;
    m_measuresCounter = 0;
}

//---------------------------------------------------------------------------------------
void LdpAnalyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pOldTiesBuilder;
    delete m_pBeamsBuilder;
    delete m_pOldBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* LdpAnalyser::analyse_tree_and_get_object(LdpTree* tree)
{
    delete_relation_builders();
    m_pTiesBuilder = LOMSE_NEW TiesBuilder(m_reporter, this);
    m_pOldTiesBuilder = LOMSE_NEW OldTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = LOMSE_NEW BeamsBuilder(m_reporter, this);
    m_pOldBeamsBuilder = LOMSE_NEW OldBeamsBuilder(m_reporter, this);
    m_pTupletsBuilder = LOMSE_NEW TupletsBuilder(m_reporter, this);
    m_pSlursBuilder = LOMSE_NEW SlursBuilder(m_reporter, this);

    m_pTree = tree;
    m_curStaff = 0;
    m_curVoice = 1;
    analyse_node(tree->get_root());

    LdpElement* pRoot = tree->get_root();
    return pRoot->get_imo();
}

//---------------------------------------------------------------------------------------
ImoObj* LdpAnalyser::analyse_tree(LdpTree* tree, const string& locator)
{
    m_fileLocator = locator;
    return analyse_tree_and_get_object(tree);
}

//---------------------------------------------------------------------------------------
void LdpAnalyser::analyse_node(LdpTree::iterator itNode)
{
    analyse_node(*itNode);
}

//---------------------------------------------------------------------------------------
ImoObj* LdpAnalyser::analyse_node(LdpElement* pNode, ImoObj* pAnchor)
{
    ElementAnalyser* a = new_analyser( pNode->get_type(), pAnchor );
    a->analyse_node(pNode);
    delete a;
    return pNode->get_imo();
}

//---------------------------------------------------------------------------------------
void LdpAnalyser::add_lyric(ImoNote* pNote, ImoLyric* pLyric)
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
void LdpAnalyser::add_marging_space_for_lyrics(ImoNote* pNote, ImoLyric* pLyric)
{
    //inform Instrument about the new lyrics line for reserving space

    int iStaff = pNote->get_staff();
    bool fAbove = pLyric->get_placement() == k_placement_above;
    ImoInstrument* pInstr = get_current_instrument();
    LUnits space = 400.0f;  //4mm per lyrics line
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
            //AWARE: If m_fInstrIdRequired==true all instruments are already created
            if (m_fInstrIdRequired)
            {
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
                //postpone until next instrument is created
                m_extraMarginSpace += space;
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
void LdpAnalyser::set_lyrics_placement(int line, int placement)
{
    while (m_lyricsPlacement.size() < size_t(line))
        m_lyricsPlacement.push_back(k_placement_below);
    m_lyricsPlacement[line-1] = placement;
}

//---------------------------------------------------------------------------------------
int LdpAnalyser::get_lyrics_placement(int line)
{
    if (m_lyricsPlacement.size() < size_t(line))
        return k_placement_below;
    else
        return m_lyricsPlacement[line-1];
}

//---------------------------------------------------------------------------------------
ImoBeam* LdpAnalyser::create_beam(const list<ImoNoteRest*>& notes)
{
    //helper method for score edition
    //TODO: refactor. This method is here because it is necessary to use BeamsBuilder.
    //It is necessary to refactor so that BeamBuilder can be used without hanving to
    //create LdpAnalyser for this.

    m_pBeamsBuilder = LOMSE_NEW BeamsBuilder(m_reporter, this);

    list<ImoNoteRest*>::const_iterator it;
    int lastNote = int( notes.size() ) - 1;
    int i=0;
    for (it = notes.begin(); it != notes.end(); ++it, ++i)
    {
        //create ImoBeamDto for this note
        ImoBeamDto* pBeamDto = static_cast<ImoBeamDto*>(
                                        ImFactory::inject(k_imo_beam_dto, m_pDoc) );
        if (i == 0)
            pBeamDto->set_beam_type(0, ImoBeam::k_begin);
        else if ( i == lastNote)
            pBeamDto->set_beam_type(0, ImoBeam::k_end);
        else
            pBeamDto->set_beam_type(0, ImoBeam::k_continue);

        //associate the beam dto to the note
        pBeamDto->set_note_rest(*it);
        (*it)->set_dirty(true);

        //add it to the beams builder;
        add_relation_info(pBeamDto);
    }

    ImoBeam* pBeam = notes.front()->get_beam();

    AutoBeamer autobeamer(pBeam);
    autobeamer.do_autobeam();

    return pBeam;
}

//---------------------------------------------------------------------------------------
ImoTie* LdpAnalyser::create_tie(ImoNote* pStart, ImoNote* pEnd)
{
    //helper method for score edition
    //TODO: refactor. This method is here because it is necessary to use TiesBuilder.
    //It is necessary to refactor so that TiesBuilder can be used without having to
    //create LdpAnalyser for this.

    m_pTiesBuilder = LOMSE_NEW TiesBuilder(m_reporter, this);

    //create ImoTieDto for the notes
    ImoTieDto* pTieDto1 = static_cast<ImoTieDto*>(
                                    ImFactory::inject(k_imo_tie_dto, m_pDoc) );
    pTieDto1->set_start(true);
    ImoTieDto* pTieDto2 = static_cast<ImoTieDto*>(
                                    ImFactory::inject(k_imo_tie_dto, m_pDoc) );
    pTieDto2->set_start(false);

    //associate the tie dto objects to the notes
    pTieDto1->set_note(pStart);
    pTieDto2->set_note(pEnd);

    //add the tie dto objects to the ties builder;
    add_relation_info(pTieDto1);
    add_relation_info(pTieDto2);

    return pStart->get_tie_next();
}

//---------------------------------------------------------------------------------------
void LdpAnalyser::add_relation_info(ImoObj* pDto)
{
    // factory method to deal with all relations

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
void LdpAnalyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
    m_pOldBeamsBuilder->clear_pending_old_beams();
    m_pTupletsBuilder->clear_pending_items();

    m_lyrics.clear();
    m_lyricIndex.clear();
    m_lyricsPlacement.clear();
}

//---------------------------------------------------------------------------------------
int LdpAnalyser::set_score_version(const string& version)
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
    return m_scoreVersion;
}

//---------------------------------------------------------------------------------------
bool LdpAnalyser::to_integer(const string& text, int* pResult)
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
int LdpAnalyser::ldp_name_to_key_type(const string& value)
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
int LdpAnalyser::ldp_name_to_clef_type(const string& value)
{
    if (value == "G")
        return k_clef_G2;
    else if (value == "F4")
        return k_clef_F4;
    else if (value == "F3")
        return k_clef_F3;
    else if (value == "F5")
        return k_clef_F5;
    else if (value == "C1")
        return k_clef_C1;
    else if (value == "C2")
        return k_clef_C2;
    else if (value == "C3")
        return k_clef_C3;
    else if (value == "C4")
        return k_clef_C4;
    else if (value == "C5")
        return k_clef_C5;
    else if (value == "percussion")
        return k_clef_percussion;
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
bool LdpAnalyser::ldp_pitch_to_components(const string& pitch, int *step, int* octave,
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
int LdpAnalyser::to_step(const char& letter)
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
int LdpAnalyser::to_octave(const char& letter)
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
EAccidentals LdpAnalyser::to_accidentals(const std::string& accidentals)
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
            else if (accidentals.compare(0, 2, "=+") == 0)
                return k_natural_sharp;
            else
                return k_invalid_accidentals;
            break;

        default:
            return k_invalid_accidentals;
    }
}

//---------------------------------------------------------------------------------------
ElementAnalyser* LdpAnalyser::new_analyser(ELdpElement type, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch (type)
    {
        case k_abbrev:          return LOMSE_NEW InstrNameAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_anchorLine:      return LOMSE_NEW AnchorLineAnalyser(this, m_reporter, m_libraryScope, pAnchor);

        //accents
        case k_accent:
        case k_legato_duro:
        case k_marccato:
        case k_marccato_legato:
        case k_marccato_staccato:
        case k_marccato_staccatissimo:
        case k_mezzo_staccato:
        case k_mezzo_staccatissimo:
        case k_staccato:
        case k_staccato_duro:
        case k_staccatissimo_duro:
        case k_staccatissimo:
        case k_tenuto:
        //stress articulations
        case k_stress:
        case k_unstress:
                                return LOMSE_NEW AccentAnalyser(this, m_reporter, m_libraryScope, pAnchor);

        //jazz pitch articulations
        case k_scoop:
        case k_plop:
        case k_doit:
        case k_falloff:
                                return LOMSE_NEW AccentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        //breath marks
        case k_breath_mark:
        case k_caesura:         return LOMSE_NEW BreathMarkAnalyser(this, m_reporter, m_libraryScope, pAnchor);


        //all other elements, in alphabetical order:

        case k_barline:         return LOMSE_NEW BarlineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_beam:            return LOMSE_NEW BeamAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_bezier:          return LOMSE_NEW BezierAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_border:          return LOMSE_NEW BorderAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_chord:           return LOMSE_NEW ChordAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_clef:            return LOMSE_NEW ClefAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_content:         return LOMSE_NEW ContentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_creationMode:    return LOMSE_NEW ContentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_color:           return LOMSE_NEW ColorAnalyser(this, m_reporter, m_libraryScope);
        case k_cursor:          return LOMSE_NEW CursorAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_defineStyle:     return LOMSE_NEW DefineStyleAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_direction:       return LOMSE_NEW DirectionAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_dynamic:         return LOMSE_NEW DynamicAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_dynamics_mark:   return LOMSE_NEW DynamicsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_endPoint:        return LOMSE_NEW PointAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_fermata:         return LOMSE_NEW FermataAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_figuredBass:     return LOMSE_NEW FiguredBassAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_font:            return LOMSE_NEW FontAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goBack:          return LOMSE_NEW GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goFwd:           return LOMSE_NEW GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
#if LOMSE_COMPATIBILITY_LDP_1_5
        case k_graphic:         return LOMSE_NEW GraphicAnalyser(this, m_reporter, m_libraryScope, pAnchor);
#endif
        case k_group:           return LOMSE_NEW GroupAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_heading:         return LOMSE_NEW HeadingAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_image:           return LOMSE_NEW ImageAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_itemizedlist:    return LOMSE_NEW ListAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_infoMIDI:        return LOMSE_NEW InfoMidiAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_instrument:      return LOMSE_NEW InstrumentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_key_signature:   return LOMSE_NEW KeySignatureAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_language:        return LOMSE_NEW LanguageAnalyser(this, m_reporter, m_libraryScope);
        case k_lenmusdoc:       return LOMSE_NEW LenmusdocAnalyser(this, m_reporter, m_libraryScope);
        case k_line:            return LOMSE_NEW LineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_link:            return LOMSE_NEW LinkAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_listitem:        return LOMSE_NEW ListItemAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_lyric:           return LOMSE_NEW LyricAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_metronome:       return LOMSE_NEW MetronomeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_musicData:       return LOMSE_NEW MusicDataAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_na:              return LOMSE_NEW NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_name:            return LOMSE_NEW InstrNameAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_newSystem:       return LOMSE_NEW ControlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_note:            return LOMSE_NEW NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_opt:             return LOMSE_NEW OptAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_orderedlist:     return LOMSE_NEW ListAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageLayout:      return LOMSE_NEW PageLayoutAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageMargins:     return LOMSE_NEW PageMarginsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageSize:        return LOMSE_NEW PageSizeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_para:            return LOMSE_NEW ParagraphAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_parameter:       return LOMSE_NEW ParamAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_parts:           return LOMSE_NEW PartsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_rest:            return LOMSE_NEW NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_settings:        return LOMSE_NEW SettingsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_score:           return LOMSE_NEW ScoreAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_score_player:    return LOMSE_NEW ScorePlayerAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_size:            return LOMSE_NEW SizeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_slur:            return LOMSE_NEW SlurAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_spacer:          return LOMSE_NEW SpacerAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_staff:           return LOMSE_NEW StaffAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_symbol:          return LOMSE_NEW XxxxxxxAnalyser(this, m_reporter, m_libraryScope);
//        case k_symbolSize:      return LOMSE_NEW XxxxxxxAnalyser(this, m_reporter, m_libraryScope);
        case k_startPoint:      return LOMSE_NEW PointAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_styles:          return LOMSE_NEW StylesAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_systemLayout:    return LOMSE_NEW SystemLayoutAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_systemMargins:   return LOMSE_NEW SystemMarginsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table:           return LOMSE_NEW TableAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table_cell:      return LOMSE_NEW TableCellAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table_column:    return LOMSE_NEW TableColumnAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table_body:      return LOMSE_NEW TableBodyAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table_head:      return LOMSE_NEW TableHeadAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_table_row:       return LOMSE_NEW TableRowAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_txt:             return LOMSE_NEW TextItemAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_text:            return LOMSE_NEW TextStringAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_textbox:         return LOMSE_NEW TextBoxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_time_modification:  return LOMSE_NEW TimeModificationAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_time_signature:  return LOMSE_NEW TimeSignatureAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tie:             return LOMSE_NEW TieAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_title:           return LOMSE_NEW TitleAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tuplet:          return LOMSE_NEW TupletAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_undoData:        return LOMSE_NEW XxxxxxxAnalyser(this, m_reporter, m_libraryScope, pAnchor);

        default:
            return LOMSE_NEW NullAnalyser(this, m_reporter, m_libraryScope);
    }
}



//=======================================================================================
// TiesBuilder implementation
//=======================================================================================
void TiesBuilder::add_relation_to_staffobjs(ImoTieDto* pEndDto)
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
bool TiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void TiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
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
void TiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}


//=======================================================================================
// OldTiesBuilder implementation
//=======================================================================================
OldTiesBuilder::OldTiesBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pStartNoteTieOld(nullptr)
    , m_pOldTieParam(nullptr)
{
}

//---------------------------------------------------------------------------------------
bool OldTiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_actual_accidentals() == pEndNote->get_actual_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::error_invalid_tie_old_syntax(int line)
{
    m_reporter << "Line " << line
               << ". No note found to match old syntax tie. Tie ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::start_old_tie(ImoNote* pNote, LdpElement* pOldTie)
{
    if (m_pStartNoteTieOld)
        create_tie_if_old_syntax_tie_pending(pNote);

    m_pStartNoteTieOld = pNote;
    m_pOldTieParam = pOldTie;
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::create_tie_if_old_syntax_tie_pending(ImoNote* pEndNote)
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
        error_invalid_tie_old_syntax( m_pOldTieParam->get_line_number() );
        m_pStartNoteTieOld = nullptr;
    }
    //else
    //  wait to see if it is possible to tie with next note
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::tie_notes(ImoNote* pStartNote, ImoNote* pEndNote)
{
    Document* pDoc = m_pAnalyser->get_document_being_analysed();
    ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, pDoc));
    pTie->set_tie_number( pTie->get_id() );

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
// SlursBuilder implementation
//=======================================================================================
void SlursBuilder::add_relation_to_staffobjs(ImoSlurDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoSlurDto* pStartInfo = m_matches.front();
    ImoSlur* pSlur = static_cast<ImoSlur*>(
                        ImFactory::inject(k_imo_slur, pDoc, pStartInfo->get_id()) );
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
// BeamsBuilder implementation
//=======================================================================================
void BeamsBuilder::add_relation_to_staffobjs(ImoBeamDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoBeamDto* pStartInfo = m_matches.front();
    ImoBeam* pBeam = static_cast<ImoBeam*>(
                        ImFactory::inject(k_imo_beam, pDoc, pStartInfo->get_id()) );
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
// OldBeamsBuilder implementation
//=======================================================================================
OldBeamsBuilder::OldBeamsBuilder(ostream& reporter, LdpAnalyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
{
}

//---------------------------------------------------------------------------------------
OldBeamsBuilder::~OldBeamsBuilder()
{
    clear_pending_old_beams();
}

//---------------------------------------------------------------------------------------
void OldBeamsBuilder::add_old_beam(ImoBeamDto* pInfo)
{
    m_pendingOldBeams.push_back(pInfo);
}

//---------------------------------------------------------------------------------------
void OldBeamsBuilder::clear_pending_old_beams()
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
bool OldBeamsBuilder::is_old_beam_open()
{
    return m_pendingOldBeams.size() > 0;
}

//---------------------------------------------------------------------------------------
void OldBeamsBuilder::error_no_end_old_beam(ImoBeamDto* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No matching 'g-' element for 'g+'. Beam ignored." << endl;
}

//---------------------------------------------------------------------------------------
void OldBeamsBuilder::close_old_beam(ImoBeamDto* pInfo)
{
    add_old_beam(pInfo);
    do_create_old_beam();
}

//---------------------------------------------------------------------------------------
void OldBeamsBuilder::do_create_old_beam()
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
// TupletsBuilder implementation
//=======================================================================================
void TupletsBuilder::add_relation_to_staffobjs(ImoTupletDto* pEndDto)
{
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoTupletDto* pStartDto = m_matches.front();
    m_pTuplet = ImFactory::inject_tuplet(pDoc, pStartDto);

    //loop for including the tuplet in all note/rest
    std::list<ImoTupletDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        //add tuplet to the note/rest
        ImoNoteRest* pNR = (*it)->get_note_rest();
        pNR->include_in_relation(pDoc, m_pTuplet, nullptr);

        //if not only graphical (LDP < 2.0) add time modification the note/rest
        if (!pStartDto->is_only_graphical())
            pNR->set_time_modification( m_pTuplet->get_normal_number(),
                                        m_pTuplet->get_actual_number() );
    }
}

//---------------------------------------------------------------------------------------
void TupletsBuilder::add_to_open_tuplets(ImoNoteRest* pNR)
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


}   //namespace lomse
