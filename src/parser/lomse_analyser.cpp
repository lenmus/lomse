//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include "lomse_analyser.h"

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
#include "lomse_parser.h"
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

using namespace std;

namespace lomse
{

//----------------------------------------------------------------------------------
// AutoBeamer implementation
//----------------------------------------------------------------------------------

void AutoBeamer::do_autobeam()
{
    extract_notes();
    process_notes();
}

void AutoBeamer::extract_notes()
{
    m_notes.clear();
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& noteRests
        = m_pBeam->get_related_objects();
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = noteRests.begin(); it != noteRests.end(); ++it)
    {
        ImoNote* pNote = dynamic_cast<ImoNote*>( (*it).first );
        if (pNote)
            m_notes.push_back(pNote);
    }
    //cout << "Num. note/rests in beam: " << noteRests.size() << endl;
    //cout << "NUm. notes in beam: " << m_notes.size() << endl;
}

void AutoBeamer::get_triad(int iNote)
{
    if (iNote == 0)
    {
        m_curNotePos = k_first_note;
        m_pPrevNote = NULL;
        m_pCurNote = m_notes[0];
        m_pNextNote = m_notes[1];
    }
    else if (iNote == (int)m_notes.size() - 1)
    {
        m_curNotePos = k_last_note;
        m_pPrevNote = m_pCurNote;
        m_pCurNote = m_notes[iNote];
        m_pNextNote = NULL;
    }
    else
    {
        m_curNotePos = k_middle_note;
        m_pPrevNote = m_pCurNote;
        m_pCurNote = m_notes[iNote];
        m_pNextNote = m_notes[iNote+1];
    }
}

void AutoBeamer::determine_maximum_beam_level_for_current_triad()
{
    m_nLevelPrev = (m_curNotePos == k_first_note ? -1 : m_nLevelCur);
    m_nLevelCur = get_beaming_level(m_pCurNote);
    m_nLevelNext = (m_pNextNote ? get_beaming_level(m_pNextNote) : -1);
}

void AutoBeamer::process_notes()
{
    for (int iNote=0; iNote < (int)m_notes.size(); iNote++)
    {
        get_triad(iNote);
        determine_maximum_beam_level_for_current_triad();
        compute_beam_types_for_current_note();
    }
}

void AutoBeamer::compute_beam_types_for_current_note()
{
    for (int level=0; level < 6; level++)
    {
        compute_beam_type_for_current_note_at_level(level);
    }
}

void AutoBeamer::compute_beam_type_for_current_note_at_level(int level)
{
    if (level > m_nLevelCur)
        m_pCurNote->set_beam_type(level, ImoBeam::k_none);

    else if (m_curNotePos == k_first_note)
    {
        //a) Case First note:
	    // 2.1) CurLevel > Level(i+1)   -->		Forward hook
	    // 2.2) other cases             -->		Begin

        if (level > m_nLevelNext)
            m_pCurNote->set_beam_type(level, ImoBeam::k_forward);    //2.1
        else
            m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2
    }

    else if (m_curNotePos == k_middle_note)
    {
        //b) Case Intermediate note:
	    //   2.1) CurLevel < Level(i)
	    //     2.1a) CurLevel > Level(i+1)		-->		End
	    //     2.1b) else						-->		Continue
        //
	    //   2.2) CurLevel > Level(i-1)
		//     2.2a) CurLevel > Level(i+1)		-->		Hook (fwd or bwd, depending on beat)
		//     2.2b) else						-->		Begin
        //
	    //   2.3) else [CurLevel <= Level(i-1)]
		//     2.3a) CurLevel > Level(i+1)		-->		End
		//     2.3b) else						-->		Continue

        if (level > m_nLevelCur)     //2.1) CurLevel < Level(i)
        {
            if (level < m_nLevelNext)
                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1a
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.1b
        }
        else if (level > m_nLevelPrev)       //2.2) CurLevel > Level(i-1)
        {
            if (level > m_nLevelNext)        //2.2a
            {
                //hook. Backward/Forward, depends on position in beat or on values
                //of previous beams
                int i;
                for (i=0; i < level; i++)
                {
                    if (m_pCurNote->get_beam_type(i) == ImoBeam::k_begin ||
                        m_pCurNote->get_beam_type(i) == ImoBeam::k_forward)
                    {
                        m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
                        break;
                    }
                    else if (m_pCurNote->get_beam_type(i) == ImoBeam::k_end ||
                                m_pCurNote->get_beam_type(i) == ImoBeam::k_backward)
                    {
                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                        break;
                    }
                }
                if (i == level)
                {
                    //no possible to take decision based on higher level beam values
                    //Determine it based on position in beat

                    //int nPos = m_pCurNote->GetPositionInBeat();
                    //if (nPos == lmUNKNOWN_BEAT)
                        //Unknownn time signature. Cannot determine type of hook. Use backward
                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                    //else if (nPos >= 0)
                    //    //on-beat note
                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
                    //else
                    //    //off-beat note
                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                }
            }
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2b
        }

        else   //   2.3) else [CurLevel <= Level(i-1)]
        {
            if (level > m_nLevelNext)
                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.3a
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.3b
        }
    }

    else
    {
        //c) Case Final note:
	    //   2.1) CurLevel <= Level(i-1)    -->		End
	    //   2.2) else						-->		Backward hook
        if (level <= m_nLevelPrev)
            m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1
        else
            m_pCurNote->set_beam_type(level, ImoBeam::k_backward);   //2.2
    }
}

int AutoBeamer::get_beaming_level(ImoNote* pNote)
{
    switch(pNote->get_note_type())
    {
        case k_eighth:
            return 0;
        case k_16th:
            return 1;
        case k_32th:
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



//-------------------------------------------------------------------------------------
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


//-------------------------------------------------------------------------------------
// Abstract class: any element analyser must derive from it

class ElementAnalyser
{
protected:
    ostream& m_reporter;
    Analyser* m_pAnalyser;
    LibraryScope& m_libraryScope;
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    ElementAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor=NULL)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_libraryScope(libraryScope)
        , m_pLdpFactory(libraryScope.ldp_factory())
        , m_pAnchor(pAnchor) {}
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
    void analyse_mandatory(ELdpElement type, ImoObj* pAnchor=NULL);
    bool get_optional(ELdpElement type);
    bool analyse_optional(ELdpElement type, ImoObj* pAnchor=NULL);
    void analyse_one_or_more(ELdpElement* pValid, int nValid);
    void analyse_staffobjs_options(ImoStaffObj* pSO);
    void analyse_scoreobj_options(ImoScoreObj* pSO);
    inline ImoObj* proceed(ELdpElement type, ImoObj* pAnchor) {
        return m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
    }

    //building the model
    void add_to_model(ImoObj* pImo);

    //auxiliary
    inline long get_node_id() { return m_pAnalysedNode->get_id(); }
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
        return m_pNextParam != NULL;
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
            m_pNextNextParam = NULL;
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
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
        ImoColorDto* pColor = dynamic_cast<ImoColorDto*>( pImo );
        Color color;
        if (pColor)
            color = pColor->get_color();
        delete pImo;
        return color;
    }

    float get_font_size_value()
    {
        const string& value = m_pParamToAnalyse->get_value();
        int size = static_cast<int>(value.size()) - 2;
        string points = value.substr(0, size);
        string number = m_pParamToAnalyse->get_value();
        float rNumber;
        std::istringstream iss(number);
        if ((iss >> std::dec >> rNumber).fail())
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid size '" + number + "'. Replaced by '12'.");
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
        ImoStyle* pStyle = NULL;

        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
        if (pScore)
        {
            pStyle = pScore->find_style(styleName);
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
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
        ImoPointDto* pPoint = dynamic_cast<ImoPointDto*>( pImo );
        TPoint point;
        if (pPoint)
            point = pPoint->get_point();
        delete pImo;
        return point;
    }

    //-----------------------------------------------------------------------------------
    TSize get_size_param()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
        ImoSizeDto* pSize = dynamic_cast<ImoSizeDto*>( pImo );
        TSize size;
        if (pSize)
            size = pSize->get_size();
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
        ImoStyle* pStyle = NULL;

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
    void check_visible(ImoBoxContent* pCO)
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
    void analyse_attachments(ImoStaffObj* pAnchor)
    {
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (is_auxobj(type))
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
            else
                error_invalid_param();

            move_to_next_param();
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
    ImoInlineObj* analyse_inline_object()
    {
        // { <inlineWrapper> | <link> | <textItem> | <image> | <button> }

        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (   /*type == k_inlineWrapper
                ||*/ type == k_txt
                || type == k_image
                || type == k_link
               )
            {
                return dynamic_cast<ImoInlineObj*>(
                    m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL) );
            }
            else
                error_invalid_param();

            move_to_next_param();
        }
        return NULL;
    }

};

//-------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullAnalyser : public ElementAnalyser
{
public:
    NullAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
        string name = m_pLdpFactory->get_name( m_pAnalysedNode->get_type() );
        cout << "Missing analyser for element '" << name << "'. Node ignored." << endl;
    }
};

//@-------------------------------------------------------------------------------------
//@ <anchorLine> = (anchorLine <destination-point>[<lineStyle>][<color>][<width>]
//@                            [<lineCapEnd>])
//@ <destination-point> = <location>    in Tenths
//@     i.e.: (anchorLine (dx value)(dy value)(lineStyle value)(color value)(width value))
//@

class AnchorLineAnalyser : public ElementAnalyser
{
public:
    AnchorLineAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLineStyle* pLine = static_cast<ImoLineStyle*>(
                                    ImFactory::inject(k_imo_line_style, pDoc) );
        pLine->set_start_point( TPoint(0.0f, 0.0f) );
        pLine->set_start_edge(k_edge_normal);
        pLine->set_start_style(k_cap_none);
        pLine->set_end_edge(k_edge_normal);

        // <destination-point> = <dx><dy>
        TPoint point;
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
            pLine->set_end_style( get_line_cap_param() );

        add_to_model( pLine );
    }

};

//@-------------------------------------------------------------------------------------
//@ ImoBarline StaffObj
//@ <barline> = (barline) | (barline <type>[<visible>][<location>])
//@ <type> = label: { start | end | double | simple | startRepetition |
//@                   endRepetition | doubleRepetition }

class BarlineAnalyser : public ElementAnalyser
{
public:
    BarlineAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBarline* pBarline = static_cast<ImoBarline*>(
                                    ImFactory::inject(k_imo_barline, pDoc) );
        pBarline->set_type(ImoBarline::k_simple);

        // <type> (label)
        if (get_optional(k_label))
            pBarline->set_type( get_barline_type() );

        // [<visible>][<location>]
        analyse_staffobjs_options(pBarline);

        error_if_more_elements();

        add_to_model(pBarline);
    }

protected:

    int get_barline_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = ImoBarline::k_simple;
        if (value == "simple")
            type = ImoBarline::k_simple;
        else if (value == "double")
            type = ImoBarline::k_double;
        else if (value == "start")
            type = ImoBarline::k_start;
        else if (value == "end")
            type = ImoBarline::k_end;
        else if (value == "endRepetition")
            type = ImoBarline::k_end_repetition;
        else if (value == "startRepetition")
            type = ImoBarline::k_start_repetition;
        else if (value == "doubleRepetition")
            type = ImoBarline::k_double_repetition;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown barline type '" + value + "'. 'simple' barline assumed.");
        }

        return type;
    }

};

//@-------------------------------------------------------------------------------------
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
    BeamAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBeamDto* pInfo = LOMSE_NEW ImoBeamDto( m_pAnalysedNode );

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


//@-------------------------------------------------------------------------------------
//@ <bezier> = (bezier <bezier-location>* )
//@ <bezier-location> = { (start-x num) | (start-y num) | (end-x num) | (end-y num) |
//@                       (ctrol1-x num) | (ctrol1-y num) | (ctrol2-x num) | (ctrol2-y num) }
//@ <num> = real number, in tenths

class BezierAnalyser : public ElementAnalyser
{
public:
    BezierAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoBezierInfo* pBezier = static_cast<ImoBezierInfo*>(
                                    ImFactory::inject(k_imo_bezier_info, pDoc));

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

//@-------------------------------------------------------------------------------------
//@ <border> = (border <width><lineStyle><color>)
//@     i.e.: (border (width 2.5)(lineStyle solid)(color #ff0000))

class BorderAnalyser : public ElementAnalyser
{
public:
    BorderAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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
            ImoObj* pImo = proceed(k_border, NULL);
            if (pImo)
            {
                ImoBorderDto* pBorder = dynamic_cast<ImoBorderDto*>(pImo);
                if (pBorder)
                    box.set_border(pBorder);
                delete pImo;
            }
        }

};

//@-------------------------------------------------------------------------------------
// <chord> = (chord <note>+ )

class ChordAnalyser : public ElementAnalyser
{
public:
    ChordAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoChord* pChord = static_cast<ImoChord*>( ImFactory::inject(k_imo_chord, pDoc) );

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
        if (m_pAnchor)
        {
            ImoMusicData* pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);
            if (!pMD)
                return;

            //add notes to musicData
            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& notes
                = pChord->get_related_objects();
            std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
            for (it = notes.begin(); it != notes.end(); ++it)
            {
                ImoNote* pNote = dynamic_cast<ImoNote*>( (*it).first );
                pMD->append_child_imo(pNote);
            }

            add_to_model(pChord);
        }
    }

};

//@-------------------------------------------------------------------------------------
//@ <clef> = (clef <type> [<symbolSize>] [<staffNum>] [<visible>] [<location>] )
//@ <type> = label: { G | F4 | F3 | C1 | C2 | C3 | C4 | percussion |
//@                   C5 | F5 | G1 | 8_G | G_8 | 8_F4 | F4_8 |
//@                   15_G | G_15 | 15_F4 | F4_15 }
//@ <symbolSize> = (symbolSize { full | cue | large } )

class ClefAnalyser : public ElementAnalyser
{
public:
    ClefAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoClef* pClef = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, pDoc) );

        // <type> (label)
        if (get_optional(k_label))
            pClef->set_clef_type( get_clef_type() );

        // [<symbolSize>]
        if (get_optional(k_symbolSize))
            set_symbol_size(pClef);

        // [<staffNum>][visible][<location>]
        analyse_staffobjs_options(pClef);

        error_if_more_elements();

        //set values that can be inherited
        pClef->set_staff( m_pAnalyser->get_current_staff() );

        add_to_model(pClef);
    }

 protected:

    int get_clef_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int clef = Analyser::ldp_name_to_clef_type(value);
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

//@-------------------------------------------------------------------------------------
//@ <color> = (color <rgba>}
//@ <rgba> = label: { #rrggbb | #rrggbbaa }

class ColorAnalyser : public ElementAnalyser
{
public:
    ColorAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
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

//@-------------------------------------------------------------------------------------
//@ <content> = (content { <score> | <text> | <para> | <heading> }

class ContentAnalyser : public ElementAnalyser
{
public:
    ContentAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoContent* pContent = static_cast<ImoContent*>(
                                        ImFactory::inject(k_imo_content, pDoc) );

        //node must be added to model before adding content to it, because
        //dynamic objects will generate requests to create other obejcts
        //on the fly, and this reuires taht all nodes are properly chained
        //in the tree.
        add_to_model(pContent);

        while (more_params_to_analyse())
        {
            if (! (analyse_optional(k_score, pContent)
                 || analyse_optional(k_dynamic, pContent)
                 || analyse_optional(k_itemizedlist, pContent)
                 || analyse_optional(k_heading, pContent)
                 || analyse_optional(k_orderedlist, pContent)
                 || analyse_optional(k_para, pContent)
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

//@-------------------------------------------------------------------------------------
//@ <newSystem> = (newSystem}

class ControlAnalyser : public ElementAnalyser
{
public:
    ControlAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemBreak* pCtrl = static_cast<ImoSystemBreak*>(
                                    ImFactory::inject(k_imo_system_break, pDoc) );
        add_to_model(pCtrl);
    }
};

//@-------------------------------------------------------------------------------------
//@ <cursor> = (cursor <instrNumber><staffNumber><timePos><objID>)
//@ <instrNumber> = integer number (0..n-1)
//@ <staffNumber> = integer number (0..n-1)
//@ <timePos> = float number
//@ <objID> = integer number
//@

class CursorAnalyser : public ElementAnalyser
{
public:
    CursorAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
            pCursor->set_id( get_long_value(-1L) );

        add_to_model(pCursor);
    }
};

//@-------------------------------------------------------------------------------------
//@ <defineStyle> = (defineStyle <syleName> { <styleProperty>* | <font><color> } )
//@ <styleProperty> = (property-tag value)
//@
//@ For backwards compatibility, also old syntax <font><color> is accepted as an
//@ alternative to full description using property-value pairs.
//@
//@ Examples:
//@     (defineStyle "Composer" (font "Times New Roman" 12pt normal) (color #000000))
//@     (defineStyle "Instruments" (font "Times New Roman" 14pt bold) (color #000000))
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
    DefineStyleAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

        //TODO <inherits>
        string parent = (name == "Default style" ? "" : "Default style");

        pStyle = create_style(name, parent);

        //old 1.5 syntax <font><color>
        if (analyse_optional(k_font, pStyle))
        {
            //<color>
            if (get_mandatory(k_color))
                pStyle->set_color_property(ImoStyle::k_color, get_color_param() );
        }
        else
        {
            //new 1.6 syntax <styleProperty>*
            while (more_params_to_analyse())
            {
                // color and background
                if (get_optional(k_color))
                    pStyle->set_color_property(ImoStyle::k_color, get_color_param() );
                else if (get_optional(k_background_color))
                    pStyle->set_color_property(ImoStyle::k_background_color, get_color_param() );

                // font
                else if (get_optional(k_font_name))
                    pStyle->set_string_property(ImoStyle::k_font_name, get_string_param() );
                else if (get_optional(k_font_size))
                    pStyle->set_float_property(ImoStyle::k_font_size, get_font_size_param() );
                else if (get_optional(k_font_style))
                    pStyle->set_int_property(ImoStyle::k_font_style, get_font_style() );
                else if (get_optional(k_font_weight))
                    pStyle->set_int_property(ImoStyle::k_font_weight, get_font_weight() );

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
                    pStyle->set_border_width_property( get_lenght_param() );
                else if (get_optional(k_border_width_top))
                    pStyle->set_lunits_property(ImoStyle::k_border_width_top, get_lenght_param() );
                else if (get_optional(k_border_width_right))
                    pStyle->set_lunits_property(ImoStyle::k_border_width_right, get_lenght_param() );
                else if (get_optional(k_border_width_bottom))
                    pStyle->set_lunits_property(ImoStyle::k_border_width_bottom, get_lenght_param() );
                else if (get_optional(k_border_width_left))
                    pStyle->set_lunits_property(ImoStyle::k_border_width_left, get_lenght_param() );

                // margin
                else if (get_optional(k_margin))
                    pStyle->set_margin_property( get_lenght_param() );
                else if (get_optional(k_margin_top))
                    pStyle->set_lunits_property(ImoStyle::k_margin_top, get_lenght_param() );
                else if (get_optional(k_margin_right))
                    pStyle->set_lunits_property(ImoStyle::k_margin_right, get_lenght_param() );
                else if (get_optional(k_margin_bottom))
                    pStyle->set_lunits_property(ImoStyle::k_margin_bottom, get_lenght_param() );
                else if (get_optional(k_margin_left))
                    pStyle->set_lunits_property(ImoStyle::k_margin_left, get_lenght_param() );

                // padding
                else if (get_optional(k_padding))
                    pStyle->set_padding_property( get_lenght_param() );
                else if (get_optional(k_padding_top))
                    pStyle->set_lunits_property(ImoStyle::k_padding_top, get_lenght_param() );
                else if (get_optional(k_padding_right))
                    pStyle->set_lunits_property(ImoStyle::k_padding_right, get_lenght_param() );
                else if (get_optional(k_padding_bottom))
                    pStyle->set_lunits_property(ImoStyle::k_padding_bottom, get_lenght_param() );
                else if (get_optional(k_padding_left))
                    pStyle->set_lunits_property(ImoStyle::k_padding_left, get_lenght_param() );

                //text
                else if (get_optional(k_text_decoration))
                    pStyle->set_int_property(ImoStyle::k_text_decoration, get_text_decoration() );
                else if (get_optional(k_vertical_align))
                    pStyle->set_int_property(ImoStyle::k_vertical_align, get_valign() );
                else if (get_optional(k_text_align))
                    pStyle->set_int_property(ImoStyle::k_text_align, get_text_align() );
                else if (get_optional(k_line_height))
                    pStyle->set_float_property(ImoStyle::k_line_height, get_float_param(1.5f) );

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
            return ImoStyle::k_font_normal;
        else if (value == "italic")
            return ImoStyle::k_italic;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font-style '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_normal;
        }
    }

    int get_font_weight()
    {
        const string value = get_string_param();
        if (value == "normal")
            return ImoStyle::k_font_normal;
        else if (value == "bold")
            return ImoStyle::k_bold;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font-weight '" + value + "'. Replaced by 'normal'.");
            return ImoStyle::k_font_normal;
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
        ImoStyles* pStyles = dynamic_cast<ImoStyles*>(m_pAnchor);
        ImoStyle* pDefault = NULL;
        if (pStyles)
            pDefault = pStyles->get_default_style();

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

//@-------------------------------------------------------------------------------------
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

class DynamicAnalyser : public ElementAnalyser
{
public:
    DynamicAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoDynamic* pDyn = static_cast<ImoDynamic*>(
                                    ImFactory::inject(k_imo_dynamic, pDoc) );

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

//@-------------------------------------------------------------------------------------
//@ <fermata> = (fermata <placement>[<componentOptions>*])
//@ <placement> = { above | below }

class FermataAnalyser : public ElementAnalyser
{
public:
    FermataAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
    }

    void set_placement(ImoFermata* pImo)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "above")
            pImo->set_placement(k_placement_above);
        else if (value == "below")
            pImo->set_placement(k_placement_below);
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown fermata placement '" + value + "'. Replaced by 'above'.");
            pImo->set_placement(k_placement_above);
        }
    }

};

//@-------------------------------------------------------------------------------------
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

class FiguredBassAnalyser : public ElementAnalyser
{
public:
    FiguredBassAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

            // [<componentOptions>*]
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
////        return (ImoFiguredBass*)NULL;    //error
////    }
////
////    //initialize options with default values
////    //AWARE: There can be two fblines, one starting in this FB and another
////    //one ending in it.
////    int nFBL=0;     //index to next fbline
////    lmFBLineInfo* pFBLineInfo[2];
////    pFBLineInfo[0] = (lmFBLineInfo*)NULL;
////    pFBLineInfo[1] = (lmFBLineInfo*)NULL;
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
////                            sElmName.c_str() );
////            else
////                pFBLineInfo[nFBL++] = AnalyzeFBLine(pX, pVStaff);
////        }
////        else
////            AnalysisError(pX, "[Element '%s'. Invalid parameter '%s'. Ignored.",
////                          sElmName.c_str(), sName.c_str() );
////    }
////
////    //analyze remaining optional parameters: <location>, <cursorPoint>
////	lmLDPOptionalTags oOptTags(this);
////	oOptTags.SetValid(lm_eTag_Location_x, lm_eTag_Location_y, -1);		//finish list with -1
////	lmLocation tPos = g_tDefaultPos;
////	oOptTags.AnalyzeCommonOptions(pNode, iP, pVStaff, NULL, NULL, &tPos);
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

//@-------------------------------------------------------------------------------------
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
    FontAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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
            pFont->weight = ImoStyle::k_bold;
            pFont->style = ImoStyle::k_font_normal;
        }
        else if (value == "normal")
        {
            pFont->weight = ImoStyle::k_font_normal;
            pFont->style = ImoStyle::k_font_normal;
        }
        else if (value == "italic")
        {
            pFont->weight = ImoStyle::k_font_normal;
            pFont->style = ImoStyle::k_italic;
        }
        else if (value == "bold-italic")
        {
            pFont->weight = ImoStyle::k_bold;
            pFont->style = ImoStyle::k_italic;
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font style '" + value + "'. Replaced by 'normal'.");
            pFont->weight = ImoStyle::k_font_normal;
            pFont->style = ImoStyle::k_font_normal;
        }
    }

};

//@-------------------------------------------------------------------------------------
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
    GoBackFwdAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoGoBackFwd* pImo = static_cast<ImoGoBackFwd*>(
                                ImFactory::inject(k_imo_go_back_fwd, pDoc) );
        bool fFwd = m_pAnalysedNode->is_type(k_goFwd);
        pImo->set_forward(fFwd);

        // <duration> |start | end (label) or <number>
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
                    float rTime = to_duration(figdots.noteType, figdots.dots);
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
};

//@-------------------------------------------------------------------------------------
//@ <group> = (group [<grpName>][<grpAbbrev>]<grpSymbol>[<joinBarlines>]
//@                  <instrument>+ )
//@
//@ <grpName> = <textString>
//@ <grpAbbrev> = <textString>
//@ <grpSymbol> = (symbol {none | brace | bracket} )
//@ <joinBarlines> = (joinBarlines {yes | no} )

class GroupAnalyser : public ElementAnalyser
{
public:
    GroupAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                                    ImFactory::inject(k_imo_instr_group, pDoc));


        // [<grpName>]
        analyse_optional(k_name, pGrp);

        // [<grpAbbrev>]
        analyse_optional(k_abbrev, pGrp);

        // <grpSymbol>
        if (!get_optional(k_symbol) || !set_symbol(pGrp))
        {
            error_msg("Missing or invalid group symbol. Must be 'none', 'brace' or 'bracket'. Group ignored.");
            delete pGrp;
            return;
        }

        // [<joinBarlines>]
        if (get_optional(k_joinBarlines))
            set_join_barlines(pGrp);

        // <instrument>+
        if (!more_params_to_analyse())
        {
            error_msg("Missing instruments in group!. Group ignored.");
            delete pGrp;
            return;
        }
        else
        {
            while (more_params_to_analyse())
            {
                if (!analyse_optional(k_instrument, pGrp))
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
        }

        add_to_model(pGrp);
    }

protected:

    bool set_symbol(ImoInstrGroup* pGrp)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string symbol = get_string_value();
        if (symbol == "brace")
            pGrp->set_symbol(ImoInstrGroup::k_brace);
        else if (symbol == "bracket")
            pGrp->set_symbol(ImoInstrGroup::k_bracket);
        else if (symbol == "none")
            pGrp->set_symbol(ImoInstrGroup::k_none);
        else
            return false;   //error

        return true;    //ok
    }

    void set_join_barlines(ImoInstrGroup* pGrp)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        pGrp->set_join_barlines( get_bool_value(true) );
    }

};

//@-------------------------------------------------------------------------------------
//@ <heading> = (heading <level> [<style>] <textItem>*)

class HeadingAnalyser : public ElementAnalyser
{
public:
    HeadingAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        // <level> (num)
        if (get_mandatory(k_number))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoHeading* pHeading = static_cast<ImoHeading*>(
                                        ImFactory::inject(k_imo_heading, pDoc) );
            pHeading->set_level( get_integer_value(1) );

            // [<style>]
            ImoStyle* pStyle = NULL;
            if (get_optional(k_style))
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                pStyle = get_doc_text_style( get_string_value() );
            }
            pHeading->set_style(pStyle);


            //<textItem>*
            while (more_params_to_analyse())
            {
                if (!analyse_optional(k_txt, pHeading))
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }

            add_to_model(pHeading);
        }

    }
};

//@-------------------------------------------------------------------------------------
//@ <image> = (image [<style>] <file>)
//@ <file> = (file <string>)
//@
//@
//@

class ImageAnalyser : public ElementAnalyser
{
public:
    ImageAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoImage* pImg = static_cast<ImoImage*>( ImFactory::inject(k_imo_image, pDoc) );

        // [<style>]
        ImoStyle* pStyle = NULL;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pImg->set_style(pStyle);

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
        LmbDocLocator loc(locator);
        SpImage img = ImageReader::load_image( loc.get_locator_for_image(imagename) );
        pImg->set_content(img);
        if (!img->is_ok())
            report_msg(m_pAnalysedNode->get_line_number(), "Error loading image. " + img->get_error_msg());
    }
};

//@-------------------------------------------------------------------------------------
//@ <infoMIDI> = (infoMIDI num_instr [num_channel])
//@ num_instr = integer: 1..256
//@ num_channel = integer: 1..16

class InfoMidiAnalyser : public ElementAnalyser
{
public:
    InfoMidiAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMidiInfo* pInfo = static_cast<ImoMidiInfo*>(
                                    ImFactory::inject(k_imo_midi_info, pDoc) );

        // num_instr
        if (!get_optional(k_number) || !set_instrument(pInfo))
        {
            error_msg("Missing or invalid MIDI instrument (1..256). MIDI info ignored.");
            delete pInfo;
            return;
        }

        // [num_channel]
        if (get_optional(k_number) && !set_channel(pInfo))
        {
            report_msg(m_pAnalysedNode->get_line_number(),
                        "Invalid MIDI channel (1..16). Channel info ignored.");
        }

        error_if_more_elements();

        add_to_model(pInfo);
    }

protected:

    bool set_instrument(ImoMidiInfo* pInfo)
    {
        int value = get_integer_value(0);
        if (value < 1 || value > 256)
            return false;   //error
        pInfo->set_instrument(value-1);
        return true;
    }

    bool set_channel(ImoMidiInfo* pInfo)
    {
        int value = get_integer_value(0);
        if (value < 1 || value > 16)
            return false;   //error
        pInfo->set_channel(value-1);
        return true;
    }

};

//@-------------------------------------------------------------------------------------
//@ <instrument> = (instrument [<instrName>][<instrAbbrev>][<staves>][<staff>]*
//@                            [<infoMIDI>] <musicData> )
//@ <instrName> = <textString>
//@ <instrAbbrev> = <textString>

class InstrumentAnalyser : public ElementAnalyser
{
public:
    InstrumentAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        m_pAnalyser->clear_pending_relations();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoInstrument* pInstrument = static_cast<ImoInstrument*>(
                                        ImFactory::inject(k_imo_instrument, pDoc) );

        // [<name>]
        analyse_optional(k_name, pInstrument);

        // [<abbrev>]
        analyse_optional(k_abbrev, pInstrument);

        // [<staves>]
        if (get_optional(k_staves))
            set_staves(pInstrument);

        // [<staff>]*
        while (analyse_optional(k_staff, pInstrument));

        // [<infoMIDI>]
        analyse_optional(k_infoMIDI, pInstrument);

        // <musicData>
        analyse_mandatory(k_musicData, pInstrument);

        error_if_more_elements();

        add_to_model(pInstrument);
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
            for(; nStaves > 1; --nStaves)
                pInstrument->add_staff();
        }
    }

};

//@-------------------------------------------------------------------------------------
//@ <key> = (key <type>[<staffobjOptions>*] )
//@ <type> = label: { C | G | D | A | E | B | F+ | C+ | C- | G- | D- | A- |
//@                   E- | B- | F | a | e | b | f+ | c+ | g+ | d+ | a+ | a- |
//@                   e- | b- | f | c | g | d }

class KeySignatureAnalyser : public ElementAnalyser
{
public:
    KeySignatureAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoKeySignature* pKey = static_cast<ImoKeySignature*>(
                                    ImFactory::inject(k_imo_key_signature, pDoc) );

        // <type> (label)
        if (get_optional(k_label))
            pKey->set_key_type( get_key_type() );

        // [<staffobjOptions>*]
        analyse_staffobjs_options(pKey);

        error_if_more_elements();

        add_to_model(pKey);
    }

    int get_key_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = Analyser::ldp_name_to_key_type(value);
        if (type == k_key_undefined)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown key '" + value + "'. Assumed 'C'.");
            type = k_key_C;
        }
        return type;
    }

};

//@-------------------------------------------------------------------------------------
//@ <language> = (language <languageCode> <charset>)
//@ obsolete since vers 1.6

class LanguageAnalyser : public ElementAnalyser
{
public:
    LanguageAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
    }
};

//@-------------------------------------------------------------------------------------
//@ <lenmusdoc> = (lenmusdoc <vers>[<settings>][<meta>][<styles>]<content>)
//@ <styles> = (styles [<defineStyle>*][<pageLayout>*])

class LenmusdocAnalyser : public ElementAnalyser
{
public:
    LenmusdocAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope)
        : ElementAnalyser(pAnalyser, reporter, libraryScope) {}

    void do_analysis()
    {
        ImoDocument* pImoDoc = NULL;

        // <vers>
        if (get_mandatory(k_vers))
        {
            string version = get_version();
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            pImoDoc = static_cast<ImoDocument*>(ImFactory::inject(k_imo_document, pDoc));
            pImoDoc->set_version(version);
            m_pAnalyser->save_root_imo_document(pImoDoc);
            pDoc->set_imo_doc(pImoDoc);
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

        pImoDoc->set_id( m_pAnalysedNode->get_id() );
        m_pAnalysedNode->set_imo(pImoDoc);
    }

protected:

    string get_version()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
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

//@-------------------------------------------------------------------------------------
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

class LineAnalyser : public ElementAnalyser
{
public:
    LineAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLineStyle* pStyle = static_cast<ImoLineStyle*>(
                                    ImFactory::inject(k_imo_line_style, pDoc) );

        // <startPoint>
        if (get_mandatory(k_startPoint))
            pStyle->set_start_point( get_point_param() );

        // <endPoint>
        if (get_mandatory(k_endPoint))
            pStyle->set_end_point( get_point_param() );

        // [<width>]
        if (get_optional(k_width))
            pStyle->set_width( get_width_param(1.0f) );

        // [<color>])
        if (get_optional(k_color))
            pStyle->set_color( get_color_param() );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            pStyle->set_line_style( get_line_style_param() );

        // [<startCap>]
        if (get_optional(k_lineCapStart))
            pStyle->set_start_style( get_line_cap_param() );

        // [<endCap>]
        if (get_optional(k_lineCapEnd))
            pStyle->set_end_style( get_line_cap_param() );

        ImoLine* pLine = static_cast<ImoLine*>( ImFactory::inject(k_imo_line, pDoc) );
        pLine->set_line_style(pStyle);
        add_to_model(pLine);
    }

};

//@-------------------------------------------------------------------------------------
//@ <link> = (link [<style>] <url> <inlineObject>+ )
//@ <url> = (url <strings>)
//@
//@ Example:
//@     (link (url "#TheoryHarmony_ch3.lms")(txt "Harmony exercise"))
//@

class LinkAnalyser : public ElementAnalyser
{
public:
    LinkAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoLink* pLink = static_cast<ImoLink*>( ImFactory::inject(k_imo_link, pDoc) );


        // [<style>]
        ImoStyle* pStyle = NULL;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pLink->set_style(pStyle);

        // <url>
        if (get_mandatory(k_url))
            set_url(pLink);

        // <inlineObject>+
        while( more_params_to_analyse() )
        {
            ImoInlineObj* pItem = analyse_inline_object();
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

//@-------------------------------------------------------------------------------------
//@ <list> = ("itemizedlist" | "orderedlist" [<style>] <listitem>* )
//@
class ListAnalyser : public ElementAnalyser
{
public:
    ListAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ELdpElement type = m_pAnalysedNode->get_type();

        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoList* pList = static_cast<ImoList*>(ImFactory::inject(k_imo_list, pDoc) );
        pList->set_list_type(type == k_itemizedlist ? ImoList::k_itemized
                                                    : ImoList::k_ordered);

        // [<style>]
        ImoStyle* pStyle = NULL;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pList->set_style(pStyle);

        // <listitem>*
        while (analyse_optional(k_listitem, pList));
        error_if_more_elements();

        add_to_model(pList);
    }
};

//@-------------------------------------------------------------------------------------
//@ <listitem> = (listitem [<style>] <inlineObject>+)  [same as <paragraph>]
//@
class ListItemAnalyser : public ElementAnalyser
{
public:
    ListItemAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoListItem* pListItem = static_cast<ImoListItem*>(
                                ImFactory::inject(k_imo_listitem, pDoc) );

        // [<style>]
        ImoStyle* pStyle = NULL;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pListItem->set_style(pStyle);

        // <inlineObject>+
        while( more_params_to_analyse() )
        {
            ImoInlineObj* pItem = analyse_inline_object();
            if (pItem)
                pListItem->add_item(pItem);

            move_to_next_param();
        }

        add_to_model(pListItem);
    }
};

//@-------------------------------------------------------------------------------------
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

class MetronomeAnalyser : public ElementAnalyser
{
public:
    MetronomeAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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

        // [<componentOptions>*]
        analyse_scoreobj_options(pMtr);

        error_if_more_elements();

        add_to_model(pMtr);
    }
};

//@-------------------------------------------------------------------------------------
//@ <musicData> = (musicData [{<note>|<rest>|<barline>|<chord>|<clef>|<figuredBass>|
//@                            <graphic>|<key>|<metronome>|<newSystem>|<spacer>|
//@                            <text>|<time>|<goFwd>|<goBack>}*] )
//AWARE: <graphic and <text> elements are accepted for compatibility with 1.5.
//These elements will no longer be possible. They must go attached to an spacer or
//other staffobj


class MusicDataAnalyser : public ElementAnalyser
{
public:
    MusicDataAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoMusicData* pMD = static_cast<ImoMusicData*>(
                                ImFactory::inject(k_imo_music_data, pDoc) );

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
                   || analyse_optional(k_graphic, pMD)
                   || analyse_optional(k_key_signature, pMD)
                   || analyse_optional(k_metronome, pMD)
                   || analyse_optional(k_newSystem, pMD)
                   || analyse_optional(k_spacer, pMD)
                   || analyse_optional(k_text, pMD)
                   || analyse_optional(k_time_signature, pMD)
                   || analyse_optional(k_goFwd, pMD)
                   || analyse_optional(k_goBack, pMD) ))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pMD);
    }

};

//@-------------------------------------------------------------------------------------
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

class NoteRestAnalyser : public ElementAnalyser
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
    NoteRestAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_pTieDto(NULL)
        , m_pTupletInfo(NULL)
        , m_pBeamInfo(NULL)
        , m_pSlurDto(NULL)
        , m_pFermata(NULL)
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
        ImoNoteRest* pNR = NULL;
        ImoNote* pNote = NULL;
        ImoRest* pRest = NULL;
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
            // [<noteOptions>*] = [{ <tie> | <stem> | <slur> }*]
            while (more_params_to_analyse())
            {
                if (get_optional(k_tie))
                    m_pTieDto = dynamic_cast<ImoTieDto*>( proceed(k_tie, NULL) );
                else if (get_optional(k_stem))
                    set_stem(pNote);
                else if (get_optional(k_slur))
                    m_pSlurDto = dynamic_cast<ImoSlurDto*>( proceed(k_slur, NULL) );
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
            m_pAnalyser->start_old_tie(pNote, m_pParamToAnalyse);
        else if (!fIsRest)
            m_pAnalyser->create_tie_if_old_syntax_tie_pending(pNote);

        add_tie_info(pNote);

        //tuplet
        if (fAddOldTuplet)
            set_old_tuplet(pNR);
        else if (m_pTupletInfo==NULL && m_pAnalyser->is_tuplet_open())
            add_to_current_tuplet(pNR);

        add_tuplet_info(pNR);

        //beam
        if (fAddOldBeam)
            set_beam_g(pNR);
        else if (m_pBeamInfo==NULL && m_pAnalyser->is_old_beam_open())
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
    }

protected:

    void analyse_note_rest_options(ImoNoteRest* pNR)
    {
        // { <beam> | <tuplet> | <voice> | <staffNum> | <fermata> }

        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (type == k_tuplet)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
                m_pTupletInfo = dynamic_cast<ImoTupletDto*>( pImo );
            }
            else if (type == k_fermata)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
                m_pFermata = dynamic_cast<ImoFermata*>( pImo );
            }
            else if (type == k_beam)
            {
                ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
                m_pBeamInfo = dynamic_cast<ImoBeamDto*>( pImo );
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
            if (Analyser::ldp_pitch_to_components(pitch, &step, &octave, &accidentals))
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
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
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid parameter '" + m_pParamToAnalyse->get_value()
                + "'. Ignored.");
        }
    }

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
            case k_32th:
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

//@-------------------------------------------------------------------------------------
//@ <option> = (opt <name><value>)
//@ <name> = label
//@ <value> = { number | label | string }

class OptAnalyser : public ElementAnalyser
{
public:
    OptAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

        if (fOk)
            add_to_model(pOpt);
        else
            delete pOpt;

        return fOk;
    }

    bool is_bool_option(const string& name)
    {
        return (name == "StaffLines.StopAtFinalBarline")
            || (name == "StaffLines.Hide")
            || (name == "Staff.DrawLeftBarline")
            || (name == "Score.FillPageWithEmptyStaves")
            || (name == "Score.JustifyFinalBarline");
    }

    bool is_number_long_option(const string& name)
    {
        return (name == "Staff.UpperLegerLines.Displacement")
                 || (name == "Render.SpacingMethod")
                 || (name == "Render.SpacingValue");
    }

    bool is_number_float_option(const string& name)
    {
        return (name == "Render.SpacingFactor");
    }

    bool is_string_option(const string& name)
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

//@-------------------------------------------------------------------------------------
//@ <pageLayout> = (pageLayout <pageSize><pageMargins><pageOrientation>)
//@ <pageSize> = (pageSize width height)
//@ <pageMargins> = (pageMargins left top right bottom binding)
//@ <pageOrientation> = [ "portrait" | "landscape" ]
//@ width, height, left, top right, bottom, binding = <num> in LUnits

class PageLayoutAnalyser : public ElementAnalyser
{
public:
    PageLayoutAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

//@-------------------------------------------------------------------------------------
//@ <pageMargins> = (pageMargins left top right bottom binding)     LUnits

class PageMarginsAnalyser : public ElementAnalyser
{
public:
    PageMarginsAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = dynamic_cast<ImoPageInfo*>(m_pAnchor);
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

//@-------------------------------------------------------------------------------------
//@ <pageSize> = (pageSize width height)        LUnits

class PageSizeAnalyser : public ElementAnalyser
{
public:
    PageSizeAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = dynamic_cast<ImoPageInfo*>(m_pAnchor);
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

//@-------------------------------------------------------------------------------------
//@ <paragraph> = (para [<style>] <inlineObject>*)

class ParagraphAnalyser : public ElementAnalyser
{
public:
    ParagraphAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoParagraph* pPara = static_cast<ImoParagraph*>(
                                        ImFactory::inject(k_imo_para, pDoc) );

        // [<style>]
        ImoStyle* pStyle = NULL;
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            pStyle = get_doc_text_style( get_string_value() );
        }
        pPara->set_style(pStyle);

        // <inlineObject>+
        while( more_params_to_analyse() )
        {
            ImoInlineObj* pItem = analyse_inline_object();
            if (pItem)
                pPara->add_item(pItem);

            move_to_next_param();
        }

        add_to_model(pPara);
    }
};

//@-------------------------------------------------------------------------------------
// <param> = (param <name> [<value>])
// <name> = <label>
// <value> = <string>

class ParamAnalyser : public ElementAnalyser
{
public:
    ParamAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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
                                    ImFactory::inject(k_imo_param_info, pDoc) );
        pParam->set_name(name);
        pParam->set_value(value);
        add_to_model(pParam);
    }
};

//@-------------------------------------------------------------------------------------
//@ <point> = (tag (dx value)(dy value))

class PointAnalyser : public ElementAnalyser
{
public:
    PointAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

//@-------------------------------------------------------------------------------------
//@ <settings> = (settings [<cursor>][<undoData>])

class SettingsAnalyser : public ElementAnalyser
{
public:
    SettingsAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

//@-------------------------------------------------------------------------------------
//@ <score> = (score <vers>[<language>][<undoData>][<creationMode>][<defineStyle>*]
//@                  [<title>*][<pageLayout>*][<systemLayout>*][<option>*]
//@                  {<instrument> | <group>}* )

class ScoreAnalyser : public ElementAnalyser
{
public:
    ScoreAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoScore* pScore = static_cast<ImoScore*>(ImFactory::inject(k_imo_score, pDoc));
        m_pAnalyser->set_score_being_analysed(pScore);

        // <vers>
        if (get_mandatory(k_vers))
            pScore->set_version( get_version() );

        // [<language>]
        analyse_optional(k_language);

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

        // {<instrument> | <group>}*
        if (!more_params_to_analyse())
            error_missing_element(k_instrument);
        else
        {
            while (more_params_to_analyse())
            {
                if (! (analyse_optional(k_instrument, pScore)
                    || analyse_optional(k_group) ))
                {
                    error_invalid_param();
                    move_to_next_param();
                }
            }
        }

        error_if_more_elements();

        add_to_model(pScore);
        m_pAnalyser->set_score_being_analysed(NULL);
    }

protected:

    string get_version()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
    }

//bool lmLDPParser::AnalyzeCreationMode(lmLDPNode* pNode, ImoScore* pScore)
//{
//    // <creationMode> = (creationMode <modeName><modeVersion>)
//
//    //Returns true if success.
//
//    wxASSERT(GetNodeName(pNode) == "creationMode");
//
//    //check that two parameters are specified
//    if(GetNodeNumParms(pNode) != 2) {
//        AnalysisError(
//            pNode,
//            "Element '%s' has less parameters than the minimum required. Element ignored.",
//            GetNodeName(pNode).c_str() );
//        return false;
//    }
//
//    //get the mode info
//    std::string sModeName = GetNodeName(pNode->GetParameter(1));
//    std::string sModeVers = GetNodeName(pNode->GetParameter(2));
//
//    //transfer to the score
//    pScore->SetCreationMode(sModeName, sModeVers);
//
//    return true;
//}

};

//@-------------------------------------------------------------------------------------
//@ <scorePlayer> = (scorePlayer <opt>+ )
//@ <file> = (file <string>)
//@
//@
//@

class ScorePlayerAnalyser : public ElementAnalyser
{
public:
    ScorePlayerAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
//        Document* pDoc = m_pAnalyser->get_document_being_analysed();
//        ImoImage* pImg = static_cast<ImoImage*>( ImFactory::inject(k_imo_image, pDoc) );
//
////        // [<style>]
////        ImoStyle* pStyle = NULL;
////        if (get_optional(k_style))
////        {
////            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
////            pStyle = get_doc_text_style( get_string_value() );
////        }
////        pHeading->set_style(pStyle);
//
//        // <file>
//        if (get_mandatory(k_file))
//        {
//            LdpElement* pValue = m_pParamToAnalyse->get_first_child();
//            pImg->set_file( pValue->get_value() );
//        }
//
//        add_to_model(pImg);
    }
};

//@-------------------------------------------------------------------------------------
//@ <size> = (size <width><height>)
//@ <width> = (width number)        value in LUnits
//@ <height> = (height number)      value in LUnits
//@     i.e.; (size (width 160)(height 100.7))

class SizeAnalyser : public ElementAnalyser
{
public:
    SizeAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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


//@-------------------------------------------------------------------------------------
//@ <slur> = (slur num <slurType>[<bezier>][color] )   ;num = slur number. integer
//@ <slurType> = { start | continue | stop }
//@
//@ Example:
//@     (slur 27 start (bezier (ctrol2-x -25)(start-y 36.765)) )
//@

class SlurAnalyser : public ElementAnalyser
{
public:
    SlurAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSlurDto* pInfo = LOMSE_NEW ImoSlurDto();

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
            pInfo->set_slur_type(ImoSlurData::k_start);
        else if (value == "stop")
            pInfo->set_slur_type(ImoSlurData::k_stop);
        else if (value == "continue")
            pInfo->set_slur_type(ImoSlurData::k_continue);
        else
            return false;   //error
        return true;    //ok
    }
};

//@-------------------------------------------------------------------------------------
//@ ImoSpacer StaffObj
//@ <spacer> = (spacer <width>[<staffobjOptions>*][<attachments>*])     width in Tenths

class SpacerAnalyser : public ElementAnalyser
{
public:
    SpacerAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
            return;
        }

        // [<staffobjOptions>*]
        analyse_staffobjs_options(pSpacer);

        add_to_model(pSpacer);

       // [<attachments>*]
        analyse_attachments(pSpacer);
    }

};

//@-------------------------------------------------------------------------------------
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
    StaffAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStaffInfo* pInfo = static_cast<ImoStaffInfo*>(
                                    ImFactory::inject(k_imo_staff_info, pDoc) );

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

//@-------------------------------------------------------------------------------------
//@ <styles> = (styles <defineStyle>*)

class StylesAnalyser : public ElementAnalyser
{
public:
    StylesAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoStyles* pStyles = static_cast<ImoStyles*>(ImFactory::inject(k_imo_styles, pDoc));

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pStyles));

        error_if_more_elements();

        //TODO:
        //ensure_default_style_exists(pStyles);

        add_to_model(pStyles);
    }

};

//@-------------------------------------------------------------------------------------
//@ <systemLayout> = (systemLayout {first | other} <systemMargins>)

class SystemLayoutAnalyser : public ElementAnalyser
{
public:
    SystemLayoutAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoSystemInfo* pInfo = static_cast<ImoSystemInfo*>(
                                        ImFactory::inject(k_imo_system_info, pDoc));

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

//@-------------------------------------------------------------------------------------
//@ <systemMargins> = (systemMargins <leftMargin><rightMargin><systemDistance>
//@                                  <topSystemDistance>)
//@ <leftMargin>, <rightMargin>, <systemDistance>, <topSystemDistance> = number (Tenths)

class SystemMarginsAnalyser : public ElementAnalyser
{
public:
    SystemMarginsAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        //Document* pDoc = m_pAnalyser->get_document_being_analysed();
        //ImoSystemInfo dto;
        ImoSystemInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_system_info())
            pDto = dynamic_cast<ImoSystemInfo*>(m_pAnchor);
        else
            return;     //what is this for?
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
    }

};

//@-------------------------------------------------------------------------------------
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
    TextBoxAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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

        ImoTextBox* pTB = ImFactory::inject_text_box(pDoc, box);

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
        ImoObj* pImo = proceed(k_border, NULL);
        if (pImo)
        {
            ImoBorderDto* pBorder = dynamic_cast<ImoBorderDto*>(pImo);
            if (pBorder)
                box.set_border(pBorder);
            delete pImo;
        }
    }

    void set_box_text(ImoTextBox* pTB)
    {
        ImoObj* pImo = proceed(k_text, NULL);
        if (pImo)
        {
            ImoScoreText* pText = dynamic_cast<ImoScoreText*>(pImo);
            if (pText)
                pTB->set_text(pText->get_text_info());
            delete pImo;
        }
    }

    void set_anchor_line(ImoTextBox* pTB)
    {
        ImoObj* pImo = proceed(k_anchorLine, NULL);
        if (pImo)
        {
            ImoLineStyle* pLine = dynamic_cast<ImoLineStyle*>(pImo);
            if (pLine)
                pTB->set_anchor_line(pLine);
            delete pImo;
        }
    }

};

//@-------------------------------------------------------------------------------------
//@ <textItem> = (txt [<style>] [<location>] string)
//@ <style> = (style <name>)
//@     if no style is specified default style is assigned.
//@
class TextItemAnalyser : public ElementAnalyser
{
public:
    TextItemAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
    {
    }

    void do_analysis()
    {
        string styleName = "Default style";

        // [<style>]
        ImoStyle* pStyle = NULL;
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
        //        error_invalid_param();
        //}

        // <string>
        if (get_mandatory(k_string))
        {
            Document* pDoc = m_pAnalyser->get_document_being_analysed();
            ImoTextItem* pText = static_cast<ImoTextItem*>(
                                        ImFactory::inject(k_imo_text_item, pDoc) );
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

//@-------------------------------------------------------------------------------------
//@ <textString> = (<textTag> string [<alingment>] <style> [<location>])
//@ <textTag> = { name | abbrev | text }
//@ <style> = (style <name>)
//@
//@ Compatibility 1.5:
//@ <alignment> = { center | left | right }
//@     alignement just indicates the position of the anchor point: at string
//@     center, at start of string (left) or at end of string (right).
//@     Since version 1.6 anchor point is always start of string.
//@
//@ <style> is now mandatory
//@     For compatibility with 1.5, if no style is specified default style is
//@     assigned.
//@
class TextStringAnalyser : public ElementAnalyser
{
protected:
    string m_styleName;

public:
    TextStringAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
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
                                        ImFactory::inject(k_imo_score_text, pDoc));
            pText->set_text(get_string_value());
            pText->set_style(NULL);

            // [<alingment>]
            if (get_optional(k_label))
                pText->set_h_align( get_alignment_value(k_halign_left) );

            // [<style>]
            if (get_optional(k_style))
                pText->set_style( get_text_style_param(m_styleName) );

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pText->set_user_location_x( get_location_param() );
                else if (get_optional(k_dy))
                    pText->set_user_location_y( get_location_param() );
                else
                    error_invalid_param();
            }
            error_if_more_elements();

            add_to_model(pText);
        }
    }

};

class InstrNameAnalyser : public TextStringAnalyser
{
public:
    InstrNameAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                      ImoObj* pAnchor)
        : TextStringAnalyser(pAnalyser, reporter, libraryScope, pAnchor,
                             "Instrument names") {}
};


//@-------------------------------------------------------------------------------------
//@ <tie> = (tie num <tieType>[<bezier>][color] )   ;num = tie number. integer
//@ <tieType> = { start | stop }

class TieAnalyser : public ElementAnalyser
{
public:
    TieAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTieDto* pInfo = static_cast<ImoTieDto*>(
                                ImFactory::inject(k_imo_tie_dto, pDoc));

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

//@-------------------------------------------------------------------------------------
//@ <timeSignature> = (time <beats><beatType>[<visible>][<location>])
//@ <beats> = <num>
//@ <beatType> = <num>

class TimeSignatureAnalyser : public ElementAnalyser
{
public:
    TimeSignatureAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        Document* pDoc = m_pAnalyser->get_document_being_analysed();
        ImoTimeSignature* pTime = static_cast<ImoTimeSignature*>(
                                    ImFactory::inject(k_imo_time_signature, pDoc) );

        // <beats> (num)
        if (get_mandatory(k_number))
            pTime->set_beats( get_integer_value(2) );

        // <beatType> (num)
        if (get_mandatory(k_number))
            pTime->set_beat_type( get_integer_value(4) );

        // [<visible>][<location>]
        analyse_staffobjs_options(pTime);

        add_to_model(pTime);
    }

};

//@-------------------------------------------------------------------------------------
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

class TitleAnalyser : public ElementAnalyser
{
public:
    TitleAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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
                pTitle->set_style( get_text_style_param() );

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pTitle->set_user_location_x( get_location_param() );
                else if (get_optional(k_dy))
                    pTitle->set_user_location_y( get_location_param() );
                else
                    error_invalid_param();

                move_to_next_param();
            }
            error_if_more_elements();

            add_to_model(pTitle);
        }
    }

};

//@-------------------------------------------------------------------------------------
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

class TupletAnalyser : public ElementAnalyser
{
public:
    TupletAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
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

        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    void set_default_values(ImoTupletDto* pInfo)
    {
        pInfo->set_show_bracket( m_pAnalyser->get_current_show_tuplet_bracket() );
        pInfo->set_placement(k_placement_default);
    }

    void set_tuplet_id(ImoTupletDto* pInfo)
    {
        //TODO. For now tuplet id is not needed. Perhaps when implementing nested
        //      tuplets it will have any use.
        get_integer_value(0);
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

//--------------------------------------------------------------------------------
// ElementAnalyser implementation
//--------------------------------------------------------------------------------

void ElementAnalyser::analyse_node(LdpElement* pNode)
{
    m_pAnalysedNode = pNode;
    move_to_first_param();
    do_analysis();
}

bool ElementAnalyser::error_missing_element(ELdpElement type)
{
    const string& parentName =
        m_pLdpFactory->get_name( m_pAnalysedNode->get_type() );
    const string& name = m_pLdpFactory->get_name(type);
    report_msg(m_pAnalysedNode->get_line_number(),
               parentName + ": missing mandatory element '" + name + "'.");
    return false;
}

void ElementAnalyser::report_msg(int numLine, const std::stringstream& msg)
{
    report_msg(numLine, msg.str());
}

void ElementAnalyser::report_msg(int numLine, const std::string& msg)
{
    m_reporter << "Line " << numLine << ". " << msg << endl;
}

bool ElementAnalyser::get_mandatory(ELdpElement type)
{
    if (!more_params_to_analyse())
    {
        error_missing_element(type);
        return NULL;
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

void ElementAnalyser::analyse_mandatory(ELdpElement type, ImoObj* pAnchor)
{
    if (get_mandatory(type))
        m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
}

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

bool ElementAnalyser::analyse_optional(ELdpElement type, ImoObj* pAnchor)
{
    if (get_optional(type))
    {
        m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
        return true;
    }
    return false;
}

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

bool ElementAnalyser::contains(ELdpElement type, ELdpElement* pValid, int nValid)
{
    for (int i=0; i < nValid; i++, pValid++)
        if (*pValid == type) return true;
    return false;
}

void ElementAnalyser::error_invalid_param()
{
    ELdpElement type = m_pParamToAnalyse->get_type();
    string name = m_pLdpFactory->get_name(type);
    if (name == "label")
        name += ":" + m_pParamToAnalyse->get_value();
    report_msg(m_pParamToAnalyse->get_line_number(),
        "Element '" + name + "' unknown or not possible here. Ignored.");
}

void ElementAnalyser::error_msg(const string& msg)
{
    report_msg(m_pAnalysedNode->get_line_number(), msg);
}

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

void ElementAnalyser::analyse_staffobjs_options(ImoStaffObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <staffobjOptions> = { <numStaff> | <componentOptions> }
    //@ <numStaff> = pn

    // [<numStaff>]
    if (get_optional(k_label))
    {
        char type = (m_pParamToAnalyse->get_value())[0];
        if (type == 'p')
            get_num_staff();
        else
            error_invalid_param();
    }

    //set staff: either found value or inherited one
    pSO->set_staff( m_pAnalyser->get_current_staff() );

    analyse_scoreobj_options(pSO);
}

void ElementAnalyser::analyse_scoreobj_options(ImoScoreObj* pSO)
{
    //@----------------------------------------------------------------------------
    //@ <componentOptions> = { <visible> | <location> | <color> }
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
    int ldpNodeType = m_pAnalysedNode->get_type();
    //pImo->set_id(m_pAnalysedNode->get_id());        //transfer id
    Linker linker( m_pAnalyser->get_document_being_analysed() );
    ImoObj* pObj = linker.add_child_to_model(m_pAnchor, pImo, ldpNodeType);
    m_pAnalysedNode->set_imo(pObj);
}




//=======================================================================================
// Analyser implementation
//=======================================================================================
Analyser::Analyser(ostream& reporter, LibraryScope& libraryScope, Document* pDoc)
    : m_reporter(reporter)
    , m_libraryScope(libraryScope)
    , m_pDoc(pDoc)
    , m_pLdpFactory(libraryScope.ldp_factory())
    , m_pTiesBuilder(NULL)
    , m_pOldTiesBuilder(NULL)
    , m_pBeamsBuilder(NULL)
    , m_pOldBeamsBuilder(NULL)
    , m_pTupletsBuilder(NULL)
    , m_pSlursBuilder(NULL)
    , m_pScore(NULL)
    , m_pImoDoc(NULL)
    , m_pTree(NULL)
    , m_fileLocator("")
    , m_nShowTupletBracket(k_yesno_default)
    , m_nShowTupletNumber(k_yesno_default)
    , m_pLastNote(NULL)
{
}

//---------------------------------------------------------------------------------------
Analyser::~Analyser()
{
    delete_relation_builders();
}

//---------------------------------------------------------------------------------------
void Analyser::delete_relation_builders()
{
    delete m_pTiesBuilder;
    delete m_pOldTiesBuilder;
    delete m_pBeamsBuilder;
    delete m_pOldBeamsBuilder;
    delete m_pTupletsBuilder;
    delete m_pSlursBuilder;
}

//---------------------------------------------------------------------------------------
ImoObj* Analyser::analyse_tree_and_get_object(LdpTree* tree)
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
InternalModel* Analyser::analyse_tree(LdpTree* tree, const string& locator)
{
    m_fileLocator = locator;
    ImoObj* pRoot = analyse_tree_and_get_object(tree);
    return LOMSE_NEW InternalModel( pRoot );
}

//---------------------------------------------------------------------------------------
void Analyser::analyse_node(LdpTree::iterator itNode)
{
    analyse_node(*itNode);
}

//---------------------------------------------------------------------------------------
ImoObj* Analyser::analyse_node(LdpElement* pNode, ImoObj* pAnchor)
{
    ElementAnalyser* a = new_analyser( pNode->get_type(), pAnchor );
    a->analyse_node(pNode);
    delete a;
    return pNode->get_imo();
}

//---------------------------------------------------------------------------------------
void Analyser::add_relation_info(ImoObj* pDto)
{
    // factory method to deal withh all relations

    if (pDto->is_tie_dto())
        m_pTiesBuilder->add_item_info(dynamic_cast<ImoTieDto*>(pDto));
    else if (pDto->is_slur_dto())
        m_pSlursBuilder->add_item_info(dynamic_cast<ImoSlurDto*>(pDto));
    else if (pDto->is_beam_dto())
        m_pBeamsBuilder->add_item_info(dynamic_cast<ImoBeamDto*>(pDto));
    else if (pDto->is_tuplet_dto())
        m_pTupletsBuilder->add_item_info(dynamic_cast<ImoTupletDto*>(pDto));
}

//---------------------------------------------------------------------------------------
void Analyser::clear_pending_relations()
{
    m_pTiesBuilder->clear_pending_items();
    m_pSlursBuilder->clear_pending_items();
    m_pBeamsBuilder->clear_pending_items();
    m_pOldBeamsBuilder->clear_pending_old_beams();
    m_pTupletsBuilder->clear_pending_items();
}

//---------------------------------------------------------------------------------------
int Analyser::ldp_name_to_key_type(const string& value)
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
int Analyser::ldp_name_to_clef_type(const string& value)
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
    else
        return k_clef_undefined;
}

//---------------------------------------------------------------------------------------
bool Analyser::ldp_pitch_to_components(const string& pitch, int *step, int* octave,
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
int Analyser::to_step(const char& letter)
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
int Analyser::to_octave(const char& letter)
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
EAccidentals Analyser::to_accidentals(const std::string& accidentals)
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
ElementAnalyser* Analyser::new_analyser(ELdpElement type, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch (type)
    {
        case k_abbrev:          return LOMSE_NEW InstrNameAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_anchorLine:      return LOMSE_NEW AnchorLineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
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
        case k_endPoint:        return LOMSE_NEW PointAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_dynamic:         return LOMSE_NEW DynamicAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_fermata:         return LOMSE_NEW FermataAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_figuredBass:     return LOMSE_NEW FiguredBassAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_font:            return LOMSE_NEW FontAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goBack:          return LOMSE_NEW GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goFwd:           return LOMSE_NEW GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_graphic:         return LOMSE_NEW XxxxxxxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
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
        case k_txt:             return LOMSE_NEW TextItemAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_text:            return LOMSE_NEW TextStringAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_textbox:         return LOMSE_NEW TextBoxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
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
void TiesBuilder::add_relation_to_notes_rests(ImoTieDto* pEndDto)
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

//---------------------------------------------------------------------------------------
void TiesBuilder::error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". This tie has the same number than that defined in line "
               << pExistingInfo->get_line_number()
               << ". This tie will be ignored." << endl;
}


//=======================================================================================
// OldTiesBuilder implementation
//=======================================================================================
OldTiesBuilder::OldTiesBuilder(ostream& reporter, Analyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pStartNoteTieOld(NULL)
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
        m_pStartNoteTieOld = NULL;
    }
    else if (m_pStartNoteTieOld->get_voice() == pEndNote->get_voice())
    {
        error_invalid_tie_old_syntax( m_pOldTieParam->get_line_number() );
        m_pStartNoteTieOld = NULL;
    }
    //else
    //  wait to see if it is possible to tie with next note
}

//---------------------------------------------------------------------------------------
void OldTiesBuilder::tie_notes(ImoNote* pStartNote, ImoNote* pEndNote)
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
// SlursBuilder implementation
//=======================================================================================
void SlursBuilder::add_relation_to_notes_rests(ImoSlurDto* pEndInfo)
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
// BeamsBuilder implementation
//=======================================================================================
void BeamsBuilder::add_relation_to_notes_rests(ImoBeamDto* pEndInfo)
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
// OldBeamsBuilder implementation
//=======================================================================================
OldBeamsBuilder::OldBeamsBuilder(ostream& reporter, Analyser* pAnalyser)
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
void TupletsBuilder::add_relation_to_notes_rests(ImoTupletDto* pEndDto)
{
    m_matches.push_back(pEndDto);
    Document* pDoc = m_pAnalyser->get_document_being_analysed();

    ImoTupletDto* pStartDto = m_matches.front();
    ImoTuplet* pTuplet = ImFactory::inject_tuplet(pDoc, pStartDto);

    std::list<ImoTupletDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoTupletData* pData = ImFactory::inject_tuplet_data(pDoc, *it);
        pNR->include_in_relation(pDoc, pTuplet, pData);
    }
}


}   //namespace lomse
