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
#include "lomse_basic_objects.h"
#include "lomse_linker.h"
#include "lomse_injectors.h"
#include "lomse_events.h"

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
    void analyse_staffobjs_options(DtoStaffObj& dto);
    void analyse_scoreobj_options(DtoScoreObj& dto);
    inline ImoObj* proceed(ELdpElement type, ImoObj* pAnchor) {
        return m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
    }

    //building the model
    void add_to_model(ImoObj* pImo);
//    void add_chord_to_model(ImoChord* pChord) {
//        m_pAnalyser->add_chord(pChord);
//    }

    //auxiliary
    inline long get_node_id() { return m_pAnalysedNode->get_id(); }
    bool contains(ELdpElement type, ELdpElement* pValid, int nValid);

    //-----------------------------------------------------------------------------------
    inline void notify_user_about(EventInfo& event)
    {
        m_libraryScope.notify_user_about(event);
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
    Color get_color_value()
    {
        ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
        ImoColorDto* pColor = dynamic_cast<ImoColorDto*>( pImo );
        Color color;
        if (pColor)
            color = pColor->get_color();
        delete pImo;
        return color;
    }

    //-----------------------------------------------------------------------------------
    TPoint get_point_value()
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
    TSize get_size_value()
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
    float get_location_value()
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(0.0f);
    }

    //-----------------------------------------------------------------------------------
    float get_width_value(float rDefault=1.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(rDefault);
    }

    //-----------------------------------------------------------------------------------
    float get_height_value(float rDefault=1.0f)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        return get_float_value(rDefault);
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
    ImoTextStyleInfo* get_text_style_value(const string& defaulName="Default style")
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string styleName = get_string_value();
        ImoTextStyleInfo* pStyle = NULL;

        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
        if (pScore)
        {
            pStyle = pScore->get_style_info(styleName);
            if (!pStyle)
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pScore->get_style_info_or_defaults(defaulName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    ImoTextStyleInfo* get_doc_text_style(const string& styleName)
    {
        ImoTextStyleInfo* pStyle = NULL;

        ImoDocument* pDoc = m_pAnalyser->get_document_being_analysed();
        if (pDoc)
        {
            pStyle = pDoc->get_style_info(styleName);
            if (!pStyle)
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Style '" + styleName + "' is not defined. Default style will be used.");
                pStyle = pDoc->get_style_info_or_defaults(styleName);
            }
        }

        return pStyle;
    }

    //-----------------------------------------------------------------------------------
    ELineStyle get_line_style_value()
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
    ELineCap get_line_cap_value()
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
    void check_visible(ImoBoxObj* pCO)
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
        ImoLineStyle line;
        line.set_start_point( TPoint(0.0f, 0.0f) );
        line.set_start_edge(k_edge_normal);
        line.set_start_style(k_cap_none);
        line.set_end_edge(k_edge_normal);

        // <destination-point> = <dx><dy>
        TPoint point;
        if (get_mandatory(k_dx))
            point.x = get_location_value();
        if (get_mandatory(k_dy))
            point.y = get_location_value();
        line.set_end_point( point );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            line.set_line_style( get_line_style_value() );

        // [<color>])
        if (get_optional(k_color))
            line.set_color( get_color_value() );

        // [<width>]
        if (get_optional(k_width))
            line.set_width( get_width_value(1.0f) );

        // [<lineCapEnd>])
        if (get_optional(k_lineCapEnd))
            line.set_end_style( get_line_cap_value() );

        add_to_model( new ImoLineStyle(line) );
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
        DtoBarline dto(ImoBarline::k_simple);

        // <type> (label)
        if (get_optional(k_label))
            dto.set_barline_type( get_barline_type() );

        // [<visible>][<location>]
        analyse_staffobjs_options(dto);

        error_if_more_elements();

        add_to_model( new ImoBarline(dto, get_node_id()) );
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
        ImoBeamDto* pInfo = new ImoBeamDto( m_pAnalysedNode );

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
        ImoBezierInfo* pBezier = new ImoBezierInfo();

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
        ImoBorderDto border;

        // <width>
        if (get_mandatory(k_width))
            border.set_width( get_width_value(1.0f) );

        // <lineStyle>
        if (get_mandatory(k_lineStyle))
            border.set_style( get_line_style_value() );

        // <color>
        if (get_mandatory(k_color))
            border.set_color( get_color_value() );

        add_to_model( new ImoBorderDto(border) );
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
        ImoChord* pChord = new ImoChord();

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
                pMD->append_child(pNote);
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
        DtoClef dto(ImoClef::k_G2);

        // <type> (label)
        if (get_optional(k_label))
            dto.set_clef_type( get_clef_type() );

        // [<symbolSize>]
        if (get_optional(k_symbolSize))
            set_symbol_size(dto);

        // [<staffNum>][visible][<location>]
        analyse_staffobjs_options(dto);

        error_if_more_elements();

        //set values that can be inherited
        dto.set_staff( m_pAnalyser->get_current_staff() );

        add_to_model( new ImoClef(dto) );
    }

 protected:

    int get_clef_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = ImoClef::k_G2;
        if (value == "G")
            type = ImoClef::k_G2;
        else if (value == "F4")
            type = ImoClef::k_F4;
        else if (value == "F3")
            type = ImoClef::k_F3;
        else if (value == "C1")
            type = ImoClef::k_C1;
        else if (value == "C2")
            type = ImoClef::k_C2;
        else if (value == "C3")
            type = ImoClef::k_C3;
        else if (value == "C4")
            type = ImoClef::k_C4;
        else if (value == "percussion")
            type = ImoClef::k_percussion;
        else if (value == "C3")
            type = ImoClef::k_C3;
        else if (value == "C5")
            type = ImoClef::k_C5;
        else if (value == "F5")
            type = ImoClef::k_F5;
        else if (value == "G1")
            type = ImoClef::k_G1;
        else if (value == "8_G")
            type = ImoClef::k_8_G2;
        else if (value == "G_8")
            type = ImoClef::k_G2_8;
        else if (value == "8_F4")
            type = ImoClef::k_8_F4;
        else if (value == "F4_8")
            type = ImoClef::k_F4_8;
        else if (value == "15_G")
            type = ImoClef::k_15_G2;
        else if (value == "G_15")
            type = ImoClef::k_G2_15;
        else if (value == "15_F4")
            type = ImoClef::k_15_F4;
        else if (value == "F4_15")
            type = ImoClef::k_F4_15;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown clef type '" + value + "'. Assumed 'G'.");
        }

        return type;
    }

    void set_symbol_size(DtoClef& dto)
    {
        const std::string& value = m_pParamToAnalyse->get_parameter(1)->get_value();
        if (value == "cue")
            dto.set_symbol_size(k_size_cue);
        else if (value == "full")
            dto.set_symbol_size(k_size_full);
        else if (value == "large")
            dto.set_symbol_size(k_size_large);
        else
        {
            dto.set_symbol_size(k_size_full);
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
        ImoColorDto* pColor = new ImoColorDto();
        m_pAnalysedNode->set_imo(pColor);
        std::string value = get_string_value();
        pColor->get_from_string(value);
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
        ImoContent* pContent = new ImoContent();

        while (more_params_to_analyse())
        {
            if (! (analyse_optional(k_score, pContent)
                 || analyse_optional(k_dynamic, pContent)
                 || analyse_optional(k_heading, pContent)
                 || analyse_optional(k_para, pContent)
                 || analyse_optional(k_text, pContent)
               ))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        error_if_more_elements();

        add_to_model(pContent);
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
        add_to_model( new ImoControl() );
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
        ImoCursorInfo* pCursor = new ImoCursorInfo();

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
//@ <defineStyle> = (defineStyle <syle_name><font><color>)
//@
//@ Examples:
//@     (defineStyle "Composer" (font "Times New Roman" 12pt normal) (color #000000))
//@     (defineStyle "Instruments" (font "Times New Roman" 14pt bold) (color #000000))

class DefineStyleAnalyser : public ElementAnalyser
{
public:
    DefineStyleAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoTextStyleInfo dto;

        //<style_name>
        if (get_mandatory(k_string))
            dto.set_name( get_string_value() );
        else
            return;

        //<font>
        analyse_mandatory(k_font, &dto);

        //<color>
        if (get_mandatory(k_color))
            dto.set_color( get_color_value() );

        error_if_more_elements();

        add_to_model( new ImoTextStyleInfo(dto) );
    }

};

//@-------------------------------------------------------------------------------------
// <dynamic> = (dynamic ... )

class DynamicAnalyser : public ElementAnalyser
{
public:
    DynamicAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoObj* pDyn = new ImoDynamic();

        ImoDocument* pDoc = m_pAnalyser->get_document_being_analysed();
        EventDynamic event(k_dynamic_content_event, pDoc, pDyn);   //, k_analyser
        m_libraryScope.notify_user_about(event);

        pDyn = event.get_object();
        add_to_model(pDyn);
    }

protected:
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
        DtoFermata dto;

        // <placement>
        if (get_mandatory(k_label))
            set_placement(dto);

        // [<componentOptions>*]
        analyse_scoreobj_options(dto);

        error_if_more_elements();

        add_to_model( new ImoFermata(dto) );
    }

    void set_placement(DtoFermata& dto)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "above")
            dto.set_placement(k_placement_above);
        else if (value == "below")
            dto.set_placement(k_placement_below);
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown fermata placement '" + value + "'. Replaced by 'above'.");
            dto.set_placement(k_placement_above);
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

            add_to_model( new ImoFiguredBass(info) );
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
        ImoFontInfo font;

        //<font_name> = string
        if (get_mandatory(k_string))
            font.name = get_string_value();

        //<font_size> = num
        if (get_optional(k_number))
            font.size = get_float_value(8.0f);
        //<font_size>. Compatibility 1.5
        else if (get_mandatory(k_label))
            font.size = get_font_size();

        //<font_style>
        if (get_mandatory(k_label))
            set_font_style_weight(font);

        add_to_model( new ImoFontInfo(font) );
    }

    float get_font_size()
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
                "Invalid size '" + number + "'. Replaced by '10'.");
            return 10.0f;
        }
        else
            return rNumber;
    }

    void set_font_style_weight(ImoFontInfo& font)
    {
        const string& value = m_pParamToAnalyse->get_value();
        if (value == "bold")
        {
            font.weight = ImoFontInfo::k_bold;
            font.style = ImoFontInfo::k_normal;
        }
        else if (value == "normal")
        {
            font.weight = ImoFontInfo::k_normal;
            font.style = ImoFontInfo::k_normal;
        }
        else if (value == "italic")
        {
            font.weight = ImoFontInfo::k_normal;
            font.style = ImoFontInfo::k_italic;
        }
        else if (value == "bold-italic")
        {
            font.weight = ImoFontInfo::k_bold;
            font.style = ImoFontInfo::k_italic;
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown font style '" + value + "'. Replaced by 'normal'.");
            font.weight = ImoFontInfo::k_normal;
            font.style = ImoFontInfo::k_normal;
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
        bool fFwd = m_pAnalysedNode->is_type(k_goFwd);
        DtoGoBackFwd dto(fFwd);

        // <duration> |start | end (label) or <number>
        if (get_optional(k_label))
        {
            string duration = m_pParamToAnalyse->get_value();
            if (duration == "start")
            {
                if (!fFwd)
                    dto.set_to_start();
                else
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Element 'goFwd' has an incoherent value: go forward to start?. Element ignored.");
                    return;
                }
            }
            else if (duration == "end")
            {
                if (fFwd)
                    dto.set_to_end();
                else
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Element 'goBack' has an incoherent value: go backwards to end?. Element ignored.");
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
                    return;
                }
                else
                {
                    float rTime = to_duration(figdots.noteType, figdots.dots);
                    dto.set_time_shift(rTime);
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
                return;
            }
            else
                dto.set_time_shift(rTime);
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown duration '" + m_pParamToAnalyse->get_name() + "'. Element ignored.");
            return;
        }

        error_if_more_elements();

        add_to_model( new ImoGoBackFwd(dto) );
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
        ImoInstrGroup* pGrp = new ImoInstrGroup();

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
            ImoHeading* pHeading = new ImoHeading( get_integer_value(1) );

            //// [<style>]
            //if (get_optional(k_style))
            //    pText->set_style( get_text_style_value(m_styleName) );

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
        ImoMidiInfo dto;

        // num_instr
        if (!get_optional(k_number) || !set_instrument(dto))
        {
            error_msg("Missing or invalid MIDI instrument (1..256). MIDI info ignored.");
            return;
        }

        // [num_channel]
        if (get_optional(k_number) && !set_channel(dto))
        {
            report_msg(m_pAnalysedNode->get_line_number(),
                        "Invalid MIDI channel (1..16). Channel info ignored.");
        }

        error_if_more_elements();

        add_to_model( new ImoMidiInfo(dto) );
    }

protected:

    bool set_instrument(ImoMidiInfo& dto)
    {
        int value = get_integer_value(0);
        if (value < 1 || value > 256)
            return false;   //error
        dto.set_instrument(value-1);
        return true;
    }

    bool set_channel(ImoMidiInfo& dto)
    {
        int value = get_integer_value(0);
        if (value < 1 || value > 16)
            return false;   //error
        dto.set_channel(value-1);
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

        ImoInstrument* pInstrument = new ImoInstrument();

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
        DtoKeySignature dto(ImoKeySignature::k_C);

        // <type> (label)
        if (get_optional(k_label))
            dto.set_key_type( get_key_type() );

        // [<staffobjOptions>*]
        analyse_staffobjs_options(dto);

        error_if_more_elements();

        add_to_model( new ImoKeySignature(dto) );
    }

    int get_key_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = ImoKeySignature::k_C;
        if (value == "C")
            type = ImoKeySignature::k_C;
        else if (value == "G")
            type = ImoKeySignature::k_G;
        else if (value == "D")
            type = ImoKeySignature::k_D;
        else if (value == "A")
            type = ImoKeySignature::k_A;
        else if (value == "E")
            type = ImoKeySignature::k_E;
        else if (value == "B")
            type = ImoKeySignature::k_B;
        else if (value == "F+")
            type = ImoKeySignature::k_Fs;
        else if (value == "C+")
            type = ImoKeySignature::k_Cs;
        else if (value == "C-")
            type = ImoKeySignature::k_Cf;
        else if (value == "G-")
            type = ImoKeySignature::k_Gf;
        else if (value == "D-")
            type = ImoKeySignature::k_Df;
        else if (value == "A-")
            type = ImoKeySignature::k_Af;
        else if (value == "E-")
            type = ImoKeySignature::k_Ef;
        else if (value == "B-")
            type = ImoKeySignature::k_Bf;
        else if (value == "F")
            type = ImoKeySignature::k_F;
        else if (value == "a")
            type = ImoKeySignature::k_a;
        else if (value == "e")
            type = ImoKeySignature::k_e;
        else if (value == "b")
            type = ImoKeySignature::k_b;
        else if (value == "f+")
            type = ImoKeySignature::k_fs;
        else if (value == "c+")
            type = ImoKeySignature::k_cs;
        else if (value == "g+")
            type = ImoKeySignature::k_gs;
        else if (value == "d+")
            type = ImoKeySignature::k_ds;
        else if (value == "a+")
            type = ImoKeySignature::k_as;
        else if (value == "a-")
            type = ImoKeySignature::k_af;
        else if (value == "e-")
            type = ImoKeySignature::k_ef;
        else if (value == "b-")
            type = ImoKeySignature::k_bf;
        else if (value == "f")
            type = ImoKeySignature::k_f;
        else if (value == "c")
            type = ImoKeySignature::k_c;
        else if (value == "g")
            type = ImoKeySignature::k_g;
        else if (value == "d")
            type = ImoKeySignature::k_d;
       else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown key '" + value + "'. Assumed 'C'.");
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
        ImoDocument* pDoc = NULL;

        // <vers>
        if (get_mandatory(k_vers))
        {
            string version = get_version();
            pDoc = new ImoDocument(version);
            m_pAnalyser->set_document_being_analysed(pDoc);
        }

        // [<settings>]
        analyse_optional(k_settings, pDoc);

        // [<meta>]
        analyse_optional(k_meta, pDoc);

        // [<styles>]
        if (!analyse_optional(k_styles, pDoc))
            add_default(pDoc);

        // [<pageLayout>*]
        while (analyse_optional(k_pageLayout, pDoc));

        // <content>
        analyse_mandatory(k_content, pDoc);

        error_if_more_elements();

        m_pAnalysedNode->set_imo(pDoc);
    }

protected:

    string get_version()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
    }

    void add_default(ImoDocument* pDoc)
    {
        Linker linker;
        linker.add_child_to_model(pDoc, new ImoStyles, k_styles);
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
        ImoLineStyle line;

        // <startPoint>
        if (get_mandatory(k_startPoint))
            line.set_start_point( get_point_value() );

        // <endPoint>
        if (get_mandatory(k_endPoint))
            line.set_end_point( get_point_value() );

        // [<width>]
        if (get_optional(k_width))
            line.set_width( get_width_value(1.0f) );

        // [<color>])
        if (get_optional(k_color))
            line.set_color( get_color_value() );

        // [<lineStyle>]
        if (get_optional(k_lineStyle))
            line.set_line_style( get_line_style_value() );

        // [<startCap>]
        if (get_optional(k_lineCapStart))
            line.set_start_style( get_line_cap_value() );

        // [<endCap>]
        if (get_optional(k_lineCapEnd))
            line.set_end_style( get_line_cap_value() );

        add_to_model( new ImoLine(line) );
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
        DtoMetronomeMark dto;

        // { <NoteType><TicksPerMinute> | <NoteType><NoteType> | <TicksPerMinute> }
        if (get_optional(k_label))
        {
            NoteTypeAndDots figdots = get_note_type_and_dots();
            dto.set_left_note_type( figdots.noteType );
            dto.set_left_dots( figdots.dots );

            if (get_optional(k_number))
            {
                // case 1: <NoteType><TicksPerMinute>
                dto.set_ticks_per_minute( get_integer_value(60) );
                dto.set_mark_type(ImoMetronomeMark::k_note_value);
            }
            else if (get_optional(k_label))
            {
                // case 2: <NoteType><NoteType>
                NoteTypeAndDots figdots = get_note_type_and_dots();
                dto.set_right_note_type( figdots.noteType );
                dto.set_right_dots( figdots.dots );
                dto.set_mark_type(ImoMetronomeMark::k_note_note);
            }
            else
            {
                report_msg(m_pAnalysedNode->get_line_number(),
                        "Error in metronome parameters. Replaced by '(metronome 60)'.");
                dto.set_ticks_per_minute(60);
                dto.set_mark_type(ImoMetronomeMark::k_value);
                add_to_model( new ImoMetronomeMark(dto) );
                return;
            }
        }
        else if (get_optional(k_number))
        {
            // case 3: <TicksPerMinute>
            dto.set_ticks_per_minute( get_integer_value(60) );
            dto.set_mark_type(ImoMetronomeMark::k_value);
        }
        else
        {
            report_msg(m_pAnalysedNode->get_line_number(),
                    "Missing metronome parameters. Replaced by '(metronome 60)'.");
            dto.set_ticks_per_minute(60);
            dto.set_mark_type(ImoMetronomeMark::k_value);
            add_to_model( new ImoMetronomeMark(dto) );
            return;
        }

        // [parenthesis]
        if (get_optional(k_label))
        {
            if (m_pParamToAnalyse->get_value() == "parenthesis")
                dto.set_parenthesis(true);
            else
                error_invalid_param();
        }

        // [<componentOptions>*]
        analyse_scoreobj_options(dto);

        error_if_more_elements();

        add_to_model( new ImoMetronomeMark(dto) );
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
        ImoMusicData* pMD = new ImoMusicData();

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

        DtoNote dtoNote;
        DtoRest dtoRest;
        DtoNoteRest* pDto;
        if (fIsRest)
            pDto = &dtoRest;
        else
            pDto = &dtoNote;

        if (!fIsRest)
        {
            // <pitch> (label)
            if (get_mandatory(k_label))
                set_pitch(dtoNote);
        }

        // <duration> (label)
        if (get_mandatory(k_label))
            set_duration(pDto);

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
        pDto->set_staff( m_pAnalyser->get_current_staff() );
        pDto->set_voice( m_pAnalyser->get_current_voice() );


        if (!fIsRest)
        {
            // [<noteOptions>*] = [{ <tie> | <stem> | <slur> }*]
            while (more_params_to_analyse())
            {
                if (get_optional(k_tie))
                    m_pTieDto = dynamic_cast<ImoTieDto*>( proceed(k_tie, NULL) );
                else if (get_optional(k_stem))
                    set_stem(dtoNote);
                else if (get_optional(k_slur))
                    m_pSlurDto = dynamic_cast<ImoSlurDto*>( proceed(k_slur, NULL) );
                else
                    break;
            }
        }

        // [<noteRestOptions>*]
        analyse_note_rest_options(pDto);

        // [<componentOptions>*]
        analyse_scoreobj_options(*pDto);

        // create object and add it to the model
        ImoNoteRest* pNR = NULL;
        if (fIsRest)
            pNR = new ImoRest(dtoRest);
        else
            pNR = new ImoNote(dtoNote);
        add_to_model(pNR);

        // [<attachments>*]
        analyse_attachments(pNR);

        // add fermata
        if (m_pFermata)
            pNR->add_attachment(m_pFermata);

        //create relations
        ImoNote* pNote = dynamic_cast<ImoNote*>(pNR);

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
                pChord = new ImoChord();
                pStartOfChordNote->include_in_relation(pChord);
//                add_chord_to_model(pChord);
            }

            //add current note to chord
            pNote->include_in_relation(pChord);

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

    void analyse_note_rest_options(DtoNoteRest* pDto)
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
                set_voice_element(pDto);
            }
            else if (type == k_staffNum)
            {
                set_staff_num_element(pDto);
            }
            else
                break;

            move_to_next_param();
        }
    }

    void set_pitch(DtoNote& dto)
    {
        string pitch = m_pParamToAnalyse->get_value();
        int step, octave, accidentals;
        if (ldp_pitch_to_components(pitch, &step, &octave, &accidentals))
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown note pitch '" + pitch + "'. Replaced by 'c4'.");
            dto.set_pitch(k_step_C, 4, k_no_accidentals);
        }
        else
            dto.set_pitch(step, octave, accidentals);
    }

    void set_duration(DtoNoteRest* pNR)
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

    void set_stem(DtoNote& dto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string value = get_string_value();
        if (value == "up")
            dto.set_stem_direction(k_stem_up);
        else if (value == "down")
            dto.set_stem_direction(k_stem_down);
        else
        {
            dto.set_stem_direction(k_stem_default);
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
        ImoBeamDto* pInfo = new ImoBeamDto();
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
            ImoBeamDto* pInfo = new ImoBeamDto();
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
        ImoTupletDto* pInfo = new ImoTupletDto();
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
        ImoTupletDto* pInfo = new ImoTupletDto();
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
        ImoTupletDto* pInfo = new ImoTupletDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_actual_number(actual);
        pInfo->set_normal_number(normal);
        m_pAnalyser->add_relation_info(pInfo);
    }

    void set_voice_element(DtoNoteRest* pDto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int voice = get_integer_value( m_pAnalyser->get_current_voice() );
        m_pAnalyser->set_current_voice(voice);
        pDto->set_voice(voice);
    }

    void set_staff_num_element(DtoNoteRest* pDto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int curStaff = m_pAnalyser->get_current_staff() + 1;
        int staff = get_integer_value(curStaff) - 1;
        m_pAnalyser->set_current_staff(staff);
        pDto->set_staff(staff);
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
        ImoOptionInfo* pOpt = new ImoOptionInfo(name);
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
        ImoPageInfo dto;

        // <pageSize>
        analyse_mandatory(k_pageSize, &dto);

        // <pageMargins>
        analyse_mandatory(k_pageMargins, &dto);

        // [ "portrait" | "landscape" ]
        if (!get_mandatory(k_label) || !set_orientation(dto))
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Invalid orientation. Expected 'portrait' or 'landscape'."
                    " 'portrait' assumed.");
            dto.set_portrait(true);
        }

        error_if_more_elements();

        add_to_model( new ImoPageInfo(dto) );
    }

protected:

    bool set_orientation(ImoPageInfo& dto)
    {
        // return true if ok
        string type = m_pParamToAnalyse->get_value();
        if (type == "portrait")
        {
            dto.set_portrait(true);
            return true;
        }
        else if (type == "landscape")
        {
            dto.set_portrait(false);
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
        ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = dynamic_cast<ImoPageInfo*>(m_pAnchor);
        else
            pDto = &dto;

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
        ImoPageInfo dto;
        ImoPageInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_page_info())
            pDto = dynamic_cast<ImoPageInfo*>(m_pAnchor);
        else
            pDto = &dto;

        //width
        if (get_mandatory(k_number))
            pDto->set_page_width( get_float_value(21000.0f) );

        //height
        if (get_mandatory(k_number))
            pDto->set_page_height( get_float_value(29700.0f) );
    }
};

//@-------------------------------------------------------------------------------------
//@ <paragraph> = (para [<style>] <textItem>*)

class ParagraphAnalyser : public ElementAnalyser
{
public:
    ParagraphAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor) {}

    void do_analysis()
    {
        ImoParagraph* pPara = new ImoParagraph();

        //// [<style>]
        //if (get_optional(k_style))
        //    pText->set_style( get_text_style_value(m_styleName) );

        //<textItem>*
        while (more_params_to_analyse())
        {
            if (!analyse_optional(k_txt, pPara))
            {
                error_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pPara);
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
        ImoPointDto point;

        // <dx>
        if (get_mandatory(k_dx))
            point.set_x( get_location_value() );

        // <dy>
        if (get_mandatory(k_dy))
            point.set_y( get_location_value() );

        error_if_more_elements();

        add_to_model( new ImoPointDto(point) );
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
        ImoScore* pScore = new ImoScore();
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

//bool lmLDPParser::AnalyzeCreationMode(lmLDPNode* pNode, lmScore* pScore)
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
        ImoSizeDto size;

        // <width>
        if (get_mandatory(k_width))
            size.set_width( get_width_value() );

        // <height>
        if (get_mandatory(k_height))
            size.set_height( get_height_value() );

        error_if_more_elements();

        //m_pAnalysedNode->set_imo( new ImoPointDto(point) );
        add_to_model( new ImoSizeDto(size) );
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
        ImoSlurDto* pInfo = new ImoSlurDto();

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
            pInfo->set_color( get_color_value() );

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
        DtoSpacer dto;

        // <width>
        if (get_optional(k_number))
        {
            dto.set_width( get_float_value() );
        }
        else
        {
            error_msg("Missing width for spacer. Spacer ignored.");
            return;
        }

        // [<staffobjOptions>*]
        analyse_staffobjs_options(dto);

        ImoSpacer* pSpacer = new ImoSpacer(dto);
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
        ImoStaffInfo dto;

        // num_instr
        if (!get_optional(k_number) || !set_staff_number(dto))
        {
            error_msg("Missing or invalid staff number. Staff info ignored.");
            return;
        }

        // [<staffType>]
        if (get_optional(k_staffType))
            set_staff_type(dto);

        // [<staffLines>]
        if (get_optional(k_staffLines))
            set_staff_lines(dto);

        // [<staffSpacing>]
        if (get_optional(k_staffSpacing))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            dto.set_line_spacing( get_float_value(180.0f));
        }

        //[<staffDistance>]
        if (get_optional(k_staffDistance))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            dto.set_staff_margin( get_float_value(1000.0f));
        }

        //[<lineThickness>]
        if (get_optional(k_lineThickness))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            dto.set_line_thickness( get_float_value(15.0f));
        }

        error_if_more_elements();

        add_to_model( new ImoStaffInfo(dto) );
    }

protected:

    bool set_staff_number(ImoStaffInfo& dto)
    {
        int value = get_integer_value(0);
        if (value < 1)
            return false;   //error
        dto.set_staff_number(value-1);
        return true;
    }

    void set_staff_type(ImoStaffInfo& dto)
    {
        const std::string& value = m_pParamToAnalyse->get_parameter(1)->get_value();
        if (value == "ossia")
            dto.set_staff_type(ImoStaffInfo::k_staff_ossia);
        else if (value == "cue")
            dto.set_staff_type(ImoStaffInfo::k_staff_cue);
        else if (value == "editorial")
            dto.set_staff_type(ImoStaffInfo::k_staff_editorial);
        else if (value == "regular")
            dto.set_staff_type(ImoStaffInfo::k_staff_regular);
        else if (value == "alternate")
            dto.set_staff_type(ImoStaffInfo::k_staff_alternate);
        else
            error_msg("Invalid staff type '" + value + "'. 'regular' staff assumed.");
    }

    void set_staff_lines(ImoStaffInfo& dto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);

        int value = get_integer_value(0);
        if (value < 1)
            error_msg("Invalid staff. Num lines must be greater than zero. Five assumed.");
        else
            dto.set_num_lines(value);
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
        ImoStyles* pStyles = new ImoStyles;

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle, pStyles));

        error_if_more_elements();

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
        ImoSystemInfo dto;

        // {first | other} <label>
        if (get_mandatory(k_label))
        {
            string type = m_pParamToAnalyse->get_value();
            if (type == "first")
                dto.set_first(true);
            else if (type == "other")
                dto.set_first(false);
            else
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                        "Expected 'first' or 'other' value but found '" + type
                        + "'. 'first' assumed.");
                dto.set_first(true);
            }
        }

        // <systemMargins>
        analyse_mandatory(k_systemMargins, &dto);

        error_if_more_elements();

        add_to_model( new ImoSystemInfo(dto) );
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
        ImoSystemInfo dto;
        ImoSystemInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_system_info())
            pDto = dynamic_cast<ImoSystemInfo*>(m_pAnchor);
        else
            pDto = &dto;

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
        ImoTextBlockInfo box;

        // <location>
        if (get_mandatory(k_dx))
            box.set_position_x( get_location_value() );
        if (get_mandatory(k_dy))
            box.set_position_y( get_location_value() );

        // <size>
        if (get_mandatory(k_size))
            box.set_size( get_size_value() );

        // [<bgColor>])
        if (get_optional(k_color))
            box.set_bg_color( get_color_value() );

        // [<border>]
        if (get_optional(k_border))
            set_box_border(box);

        ImoTextBox* pTB = new ImoTextBox(box);

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
protected:
    string m_styleName;

public:
    TextItemAnalyser(Analyser* pAnalyser, ostream& reporter, LibraryScope& libraryScope,
                       ImoObj* pAnchor, const string& styleName="Default txt")
        : ElementAnalyser(pAnalyser, reporter, libraryScope, pAnchor)
        , m_styleName(styleName)
    {
    }

    void do_analysis()
    {

        // [<style>]
        if (get_optional(k_style))
        {
            m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
            m_styleName = get_string_value();
        }
        else
            m_styleName = "Default style";


        //// [<location>]
        //while (more_params_to_analyse())
        //{
        //    if (get_optional(k_dx))
        //        pText->set_user_location_x( get_location_value() );
        //    else if (get_optional(k_dy))
        //        pText->set_user_location_y( get_location_value() );
        //    else if (get_optional(k_string)
        //        break;
        //    else
        //        error_invalid_param();
        //}

        // <string>
        if (get_mandatory(k_string))
        {
            ImoTextStyleInfo* pStyle = get_doc_text_style(m_styleName);
            ImoTextItem* pText = new ImoTextItem(get_string_value(), pStyle);

            error_if_more_elements();

            add_to_model(pText);
        }
    }

protected:
    //void set_default_style(ImoScoreText* pText)
    //{
    //    ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
    //    if (pScore)
    //        pText->set_style( pScore->get_style_info_or_defaults(m_styleName) );
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
            ImoScoreText* pText = new ImoScoreText(get_string_value());
            set_default_style(pText);

            // [<alingment>]
            if (get_optional(k_label))
                pText->set_h_align( get_alignment_value(k_halign_left) );

            // [<style>]
            if (get_optional(k_style))
                pText->set_style( get_text_style_value(m_styleName) );

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pText->set_user_location_x( get_location_value() );
                else if (get_optional(k_dy))
                    pText->set_user_location_y( get_location_value() );
                else
                    error_invalid_param();
            }
            error_if_more_elements();

            add_to_model(pText);
        }
    }

protected:
    void set_default_style(ImoScoreText* pText)
    {
        ImoScore* pScore = m_pAnalyser->get_score_being_analysed();
        if (pScore)
            pText->set_style( pScore->get_style_info_or_defaults(m_styleName) );
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
        ImoTieDto* pInfo = new ImoTieDto();

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
            pInfo->set_color( get_color_value() );

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
        DtoTimeSignature dto;

        // <beats> (num)
        if (get_mandatory(k_number))
            dto.set_beats( get_integer_value(2) );

        // <beatType> (num)
        if (get_mandatory(k_number))
            dto.set_beat_type( get_integer_value(4) );

        // [<visible>][<location>]
        analyse_staffobjs_options(dto);

        add_to_model( new ImoTimeSignature(dto) );
    }

};

//@-------------------------------------------------------------------------------------
//@ <title> = (title <h-alignment> string [<style>][<location>])
//@ <h-alignment> = label: {left | center | right }
//@ <style> = (style name)
//@         name = string.  Must be a style name defined with defineStyle
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
        // <h-alignment>
        int nAlignment = k_halign_left;
        if (get_mandatory(k_label))
            nAlignment = get_alignment_value(k_halign_center);

        // <string>
        if (get_mandatory(k_string))
        {
            ImoScoreTitle* pTitle = new ImoScoreTitle(get_string_value(), nAlignment);

            // [<style>]
            if (get_optional(k_style))
                pTitle->set_style( get_text_style_value() );

            // [<location>]
            while (more_params_to_analyse())
            {
                if (get_optional(k_dx))
                    pTitle->set_user_location_x( get_location_value() );
                else if (get_optional(k_dy))
                    pTitle->set_user_location_y( get_location_value() );
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
        ImoTupletDto* pInfo = new ImoTupletDto();
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

void ElementAnalyser::analyse_staffobjs_options(DtoStaffObj& dto)
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
    dto.set_staff( m_pAnalyser->get_current_staff() );

    analyse_scoreobj_options(dto);
}

void ElementAnalyser::analyse_scoreobj_options(DtoScoreObj& dto)
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
                dto.set_visible( get_bool_value(true) );
                break;
            }
            case k_color:
            {
                dto.set_color( get_color_value() );
                break;
            }
            case k_dx:
            {
                //m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                //dto.set_user_location_x( get_float_value(0.0f) );
                dto.set_user_location_x( get_location_value() );
                break;
            }
            case k_dy:
            {
                //m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                //dto.set_user_location_y( get_float_value(0.0f) );
                dto.set_user_location_y( get_location_value() );
                break;
            }
            default:
                return;
        }

        move_to_next_param();
    }
}

void ElementAnalyser::add_to_model(ImoObj* pImo)
{
    int ldpNodeType = m_pAnalysedNode->get_type();
    pImo->set_id(m_pAnalysedNode->get_id());        //transfer id
    Linker linker;
    ImoObj* pObj = linker.add_child_to_model(m_pAnchor, pImo, ldpNodeType);
    m_pAnalysedNode->set_imo(pObj);
}




//=======================================================================================
// Analyser implementation
//=======================================================================================
//Analyser::Analyser(ostream& reporter, LibraryScope& libraryScope)
//    , m_pLdpFactory(pFactory)
Analyser::Analyser(ostream& reporter, LibraryScope& libraryScope)
    : m_reporter(reporter)
    , m_libraryScope(libraryScope)
    , m_pLdpFactory(libraryScope.ldp_factory())
    , m_pTiesBuilder(NULL)
    , m_pOldTiesBuilder(NULL)
    , m_pBeamsBuilder(NULL)
    , m_pOldBeamsBuilder(NULL)
    , m_pTupletsBuilder(NULL)
    , m_pSlursBuilder(NULL)
    , m_pScore(NULL)
    , m_pDoc(NULL)
    , m_pTree(NULL)
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
    m_pTiesBuilder = new TiesBuilder(m_reporter, this);
    m_pOldTiesBuilder = new OldTiesBuilder(m_reporter, this);
    m_pBeamsBuilder = new BeamsBuilder(m_reporter, this);
    m_pOldBeamsBuilder = new OldBeamsBuilder(m_reporter, this);
    m_pTupletsBuilder = new TupletsBuilder(m_reporter, this);
    m_pSlursBuilder = new SlursBuilder(m_reporter, this);

    m_pTree = tree;
    m_curStaff = 0;
    m_curVoice = 1;
    analyse_node(tree->get_root());

    LdpElement* pRoot = tree->get_root();
    return pRoot->get_imo();
}

//---------------------------------------------------------------------------------------
InternalModel* Analyser::analyse_tree(LdpTree* tree)
{
    ImoObj* pRoot = analyse_tree_and_get_object(tree);
    return new InternalModel( pRoot );
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
ElementAnalyser* Analyser::new_analyser(ELdpElement type, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch (type)
    {
        case k_abbrev:          return new InstrNameAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_anchorLine:      return new AnchorLineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_barline:         return new BarlineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_beam:            return new BeamAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_bezier:          return new BezierAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_border:          return new BorderAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_chord:           return new ChordAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_clef:            return new ClefAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_content:         return new ContentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_creationMode:    return new ContentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_color:           return new ColorAnalyser(this, m_reporter, m_libraryScope);
        case k_cursor:          return new CursorAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_defineStyle:     return new DefineStyleAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_endPoint:        return new PointAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_dynamic:         return new DynamicAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_fermata:         return new FermataAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_figuredBass:     return new FiguredBassAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_font:            return new FontAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goBack:          return new GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_goFwd:           return new GoBackFwdAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_graphic:         return new XxxxxxxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_group:           return new GroupAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_heading:         return new HeadingAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_infoMIDI:        return new InfoMidiAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_instrument:      return new InstrumentAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_key_signature:   return new KeySignatureAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_language:        return new LanguageAnalyser(this, m_reporter, m_libraryScope);
        case k_lenmusdoc:       return new LenmusdocAnalyser(this, m_reporter, m_libraryScope);
        case k_line:            return new LineAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_metronome:       return new MetronomeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_musicData:       return new MusicDataAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_na:              return new NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_name:            return new InstrNameAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_newSystem:       return new ControlAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_note:            return new NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_opt:             return new OptAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageLayout:      return new PageLayoutAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageMargins:     return new PageMarginsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_pageSize:        return new PageSizeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_rest:            return new NoteRestAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_settings:        return new SettingsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_score:           return new ScoreAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_size:            return new SizeAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_slur:            return new SlurAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_spacer:          return new SpacerAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_staff:           return new StaffAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_symbol:          return new XxxxxxxAnalyser(this, m_reporter, m_libraryScope);
//        case k_symbolSize:      return new XxxxxxxAnalyser(this, m_reporter, m_libraryScope);
        case k_para:            return new ParagraphAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_startPoint:      return new PointAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_styles:          return new StylesAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_systemLayout:    return new SystemLayoutAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_systemMargins:   return new SystemMarginsAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_txt:             return new TextItemAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_text:            return new TextStringAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_textbox:         return new TextBoxAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_time_signature:  return new TimeSignatureAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tie:             return new TieAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_title:           return new TitleAnalyser(this, m_reporter, m_libraryScope, pAnchor);
        case k_tuplet:          return new TupletAnalyser(this, m_reporter, m_libraryScope, pAnchor);
//        case k_undoData:        return new XxxxxxxAnalyser(this, m_reporter, m_libraryScope, pAnchor);

        default:
            return new NullAnalyser(this, m_reporter, m_libraryScope);
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
            && (pStartNote->get_accidentals() == pEndNote->get_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

//---------------------------------------------------------------------------------------
void TiesBuilder::tie_notes(ImoTieDto* pStartDto, ImoTieDto* pEndDto)
{
    ImoNote* pStartNote = pStartDto->get_note();
    ImoNote* pEndNote = pEndDto->get_note();

    ImoTie* pTie = new ImoTie();
    pTie->set_tie_number( pStartDto->get_tie_number() );
    pTie->set_color( pStartDto->get_color() );

    ImoTieData* pStartData = new ImoTieData(pStartDto);
    pStartNote->include_in_relation(pTie, pStartData);

    ImoTieData* pEndData = new ImoTieData(pEndDto);
    pEndNote->include_in_relation(pTie, pEndData);

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
            && (pStartNote->get_accidentals() == pEndNote->get_accidentals())
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
    ImoTie* pTie = new ImoTie();    //( pEndInfo->get_tie_number() );

    ImoTieDto startDto;
    startDto.set_start(true);
    ImoTieData* pStartData = new ImoTieData(&startDto);
    pStartNote->include_in_relation(pTie, pStartData);

    ImoTieDto endDto;
    endDto.set_start(false);
    ImoTieData* pEndData = new ImoTieData(&endDto);
    pEndNote->include_in_relation(pTie, pEndData);

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);
}



//=======================================================================================
// SlursBuilder implementation
//=======================================================================================
void SlursBuilder::add_relation_to_notes_rests(ImoSlurDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);

    ImoSlur* pSlur = new ImoSlur( pEndInfo->get_slur_number() );
    std::list<ImoSlurDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNote* pNote = (*it)->get_note();
        ImoSlurData* pData = new ImoSlurData(*it);
        pNote->include_in_relation(pSlur, pData);
    }
}



//=======================================================================================
// BeamsBuilder implementation
//=======================================================================================
void BeamsBuilder::add_relation_to_notes_rests(ImoBeamDto* pEndInfo)
{
    m_matches.push_back(pEndInfo);

    ImoBeam* pBeam = new ImoBeam();
    std::list<ImoBeamDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoBeamData* pData = new ImoBeamData(*it);
        pNR->include_in_relation(pBeam, pData);
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
    ImoBeam* pBeam = new ImoBeam();
    std::list<ImoBeamDto*>::iterator it;
    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoBeamData* pData = new ImoBeamData(*it);
        pNR->include_in_relation(pBeam, pData);
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

    ImoTupletDto* pStartDto = m_matches.front();
    ImoTuplet* pTuplet = new ImoTuplet( pStartDto );

    std::list<ImoTupletDto*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        ImoTupletData* pData = new ImoTupletData(*it);
        pNR->include_in_relation(pTuplet, pData);
    }
}



//=======================================================================================
// ImoObjFactory implementation
//=======================================================================================
ImoObjFactory::ImoObjFactory(LibraryScope& libraryScope, ostream& reporter)
    : m_libraryScope(libraryScope)
    , m_reporter(reporter)
{
}

//---------------------------------------------------------------------------------------
ImoObj* ImoObjFactory::create_object(const std::string& ldpSource)
{
    LdpParser parser(m_reporter, m_libraryScope.ldp_factory());
    SpLdpTree tree = parser.parse_text(ldpSource);
    Analyser a(m_reporter, m_libraryScope);
    return a.analyse_tree_and_get_object(tree);
}

//---------------------------------------------------------------------------------------
ImoTextItem* ImoObjFactory::create_text_item(const std::string& ldpSource,
                                             ImoTextStyleInfo* pStyle)
{
    ImoTextItem* pText = dynamic_cast<ImoTextItem*>( create_object(ldpSource) );
    pText->set_style(pStyle);
    return pText;
}



}   //namespace lomse
