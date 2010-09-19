//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_analyser.h"

#include <iostream>
#include <sstream>
#include <locale>
#include <vector>
#include <algorithm>   // for find
#include "lomse_ldp_factory.h"
#include "lomse_tree.h"
#include "lomse_parser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_elements.h"
#include "lomse_basic_model.h"
#include "lomse_basic_objects.h"
#include "lomse_linker.h"

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
    std::list<ImoStaffObj*>& m_notesRestsInBeam = m_pBeam->get_related_objects();
    std::list<ImoStaffObj*>::iterator it;
    for (it = m_notesRestsInBeam.begin(); it != m_notesRestsInBeam.end(); ++it)
    {
        ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
        if (pNote)
            m_notes.push_back(pNote);
    }
    //cout << "Num. note/rests in beam: " << m_notesRestsInBeam.size() << endl;
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
        case ImoNoteRest::k_eighth:
            return 0;
        case ImoNoteRest::k_16th:
            return 1;
        case ImoNoteRest::k_32th:
            return 2;
        case ImoNoteRest::k_64th:
            return 3;
        case ImoNoteRest::k_128th:
            return 4;
        case ImoNoteRest::k_256th:
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
    LdpFactory* m_pLdpFactory;
    ImoObj* m_pAnchor;

public:
    ElementAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor=NULL)
        : m_reporter(reporter)
        , m_pAnalyser(pAnalyser)
        , m_pLdpFactory(pFactory)
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
    void error_if_more_elements();
    void analyse_staffobjs_options(DtoStaffObj& dto);
    void analyse_component_options(DtoComponentObj& dto);
    inline ImoObj* proceed(ELdpElement type, ImoObj* pAnchor) {
        return m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
    }

    //building the model
    void add_to_model(ImoObj* pImo);

    //auxiliary
    bool contains(ELdpElement type, ELdpElement* pValid, int nValid);
    void error_and_remove_invalid_param();
    void error_and_remove_element(const string& msg);
    void remove_this_element();

    inline bool more_params_to_analyse() {
        return m_pNextParam != NULL;
    }

    inline LdpElement* get_param_to_analyse() {
        return m_pNextParam;
    }

    inline void move_to_next_param() {
        m_pNextParam = m_pNextNextParam;
        prepare_next_one();
    }

    inline void prepare_next_one() {
        if (m_pNextParam)
            m_pNextNextParam = m_pNextParam->get_next_sibling();
        else
            m_pNextNextParam = NULL;
    }

    inline void move_to_first_param() {
        m_pNextParam = m_pAnalysedNode->get_first_child();
        prepare_next_one();
    }

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
            LdpElement* value = m_pLdpFactory->new_value(k_label, "p1");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            m_pAnalyser->set_current_staff(0);
        }
        else
            m_pAnalyser->set_current_staff(--nStaff);
    }

    bool is_long_value()
    {
        string number = m_pParamToAnalyse->get_value();
        long nNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> nNumber).fail());
    }

    long get_long_number(long nDefault=0L)
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
            LdpElement* value = m_pLdpFactory->new_value(k_number, replacement.str());
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            return nDefault;
        }
        else
            return nNumber;
    }

    int get_integer_number(int nDefault)
    {
        return static_cast<int>( get_long_number(static_cast<int>(nDefault)) );
    }

    bool is_float_value()
    {
        string number = m_pParamToAnalyse->get_value();
        float rNumber;
        std::istringstream iss(number);
        return !((iss >> std::dec >> rNumber).fail());
    }

    float get_float_number(float rDefault=0.0f)
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
            LdpElement* value = m_pLdpFactory->new_value(k_number, replacement.str());
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            return rDefault;
        }
        else
            return rNumber;
    }

    bool is_bool_value()
    {
        string value = m_pParamToAnalyse->get_value();
        return  value == "true" || value == "yes"
             || value == "false" || value == "no" ;
    }

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
            LdpElement* value = m_pLdpFactory->new_value(k_string, replacement.str());
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            return fDefault;
        }
    }

    string get_string_value()
    {
        return m_pParamToAnalyse->get_value();
    }

    void check_visible(ImoComponentObj* pCO)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "visible")
            pCO->set_visible(true);
        else if (value == "noVisible")
            pCO->set_visible(false);
        else
        {
            error_and_remove_invalid_param();
            pCO->set_visible(true);
        }
    }

    NoteTypeAndDots get_note_type_and_dots()
    {
        string duration = m_pParamToAnalyse->get_value();
        NoteTypeAndDots figdots = ldp_duration_to_components(duration);
        if (figdots.noteType == ImoNoteRest::k_unknown)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown note/rest duration '" + duration + "'. Replaced by 'q'.");
            LdpElement* value = m_pLdpFactory->new_value(k_duration, "q");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            figdots.noteType = ImoNoteRest::k_quarter;
        }
        return figdots;
    }

    void analyse_attachments(ImoStaffObj* pAnchor)
    {
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (is_auxobj(type))
                m_pAnalyser->analyse_node(m_pParamToAnalyse, pAnchor);
            else
                error_and_remove_invalid_param();

            move_to_next_param();
        }
    }

};

//-------------------------------------------------------------------------------------
// default analyser to use when there is no defined analyser for an LDP element

class NullAnalyser : public ElementAnalyser
{
public:
    NullAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory)
        : ElementAnalyser(pAnalyser, reporter, pFactory) {}

    void do_analysis()
    {
        string name = m_pLdpFactory->get_name( m_pAnalysedNode->get_type() );
        cout << "Missing analyser for element '" << name << "'. Node removed." << endl;
        remove_this_element();
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
    BarlineAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoBarline dto(ImoBarline::k_simple);

        // <type> (label)
        if (get_optional(k_label))
            dto.set_barline_type( get_barline_type() );

        // [<visible>][<location>]
        analyse_staffobjs_options(dto);

        error_if_more_elements();

        add_to_model( new ImoBarline(dto) );
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
            LdpElement* value = m_pLdpFactory->new_value(k_label, "simple");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
        }

        return type;
    }

};

//@-------------------------------------------------------------------------------------
//@ <beam> = (beam num <beamtype>+)
//@ <beamtype> = label: { begin | continue | end | forward | backward }
//@
//@ Examples:
//@     (beam 17 begin)
//@     (beam 17 continue begin forward)
//@     (beam 17 continue continue)
//@     (beam 17 end end backward)


class BeamAnalyser : public ElementAnalyser
{
public:
    BeamAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoBeamInfo* pInfo = new ImoBeamInfo( m_pAnalysedNode );

        // num
        if (get_optional(k_number))
            pInfo->set_beam_number( get_integer_number(0) );
        else
        {
            error_and_remove_element("Missing or invalid beam number. Beam ignored.");
            delete pInfo;
            return;
        }

        // <beamtype>+ (label)
        int level = 0;
        while (more_params_to_analyse())
        {
            if (!get_optional(k_label) || !set_beam_type(level++, pInfo))
            {
                error_and_remove_element("Missing or invalid beam type. Beam ignored.");
                delete pInfo;
                return;
            }
        }

        m_pAnalysedNode->set_imo(pInfo);
    }

protected:

    bool set_beam_type(int level, ImoBeamInfo* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
        if (value == "begin")
            pInfo->set_beam_type(level, ImoBeam::k_begin);
        else if (value == "continue")
            pInfo->set_beam_type(level, ImoBeam::k_continue);
        else if (value == "end")
            pInfo->set_beam_type(level, ImoBeam::k_end);
        else if (value == "forward")
            pInfo->set_beam_type(level, ImoBeam::k_forward);
        else if (value == "backward")
            pInfo->set_beam_type(level, ImoBeam::k_backward);
        else
            return false;   //error
        return true;    //ok
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
    BezierAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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
                error_and_remove_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pBezier);
    }

    void set_x_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        float value = get_float_number(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.x = value;
    }

    void set_y_in_point(int i, ImoBezierInfo* pBezier)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        float value = get_float_number(0.0f);
        TPoint& point = pBezier->get_point(i);
        point.y = value;
    }
};

//@-------------------------------------------------------------------------------------
// <chord> = (chord <note>+ )
// deprecated since 1.6

class ChordAnalyser : public ElementAnalyser
{
public:
    ChordAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoMusicData* pAuxMD = new ImoMusicData();
        m_pAnalysedNode->set_imo(pAuxMD);

        // <note>*
        while (more_params_to_analyse())
        {
            if (!analyse_optional(k_note, pAuxMD))
            {
                error_and_remove_invalid_param();
                move_to_next_param();
            }
        }

        add_to_music_data(pAuxMD);
    }

protected:

    void add_to_music_data(ImoMusicData* pChordNotes)
    {
        //get anchor musicData
        ImoMusicData* pMD;
        if (m_pAnchor)
        {
            pMD = dynamic_cast<ImoMusicData*>(m_pAnchor);
            if (!pMD)
                return;

            //transfer notes
            ImoObj::children_iterator it;
            int i = 0;
            for (it = pChordNotes->begin(); it != pChordNotes->end(); ++it, ++i)
            {
                ImoNote* pNote = dynamic_cast<ImoNote*>( *it );
                ImoNote* pClone = new ImoNote();
                *pClone = *pNote;
                pClone->set_in_chord(i > 0);
                pMD->append_child(pClone);
            }
        }
    }

};

//@-------------------------------------------------------------------------------------
//@ ImoClef StaffObj
//@ <clef> = (clef <type>[<staffNum>][<visible>][<location>] )
//@ <type> = label: { G | F4 | F3 | C1 | C2 | C3 | C4 | percussion |
//@                   C3 | C5 | F5 | G1 | 8_G | G_8 | 8_F4 | F4_8 |
//@                   15_G | G_15 | 15_F4 | F4_15 }

class ClefAnalyser : public ElementAnalyser
{
public:
    ClefAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoClef dto(ImoClef::k_G3);

        // <type> (label)
        if (get_optional(k_label))
            dto.set_clef_type( get_clef_type() );

        // [<staffNum>][visible][<location>]
        analyse_staffobjs_options(dto);

        error_if_more_elements();

        //set values that can be inherited
        dto.set_staff( m_pAnalyser->get_current_staff() );

        add_to_model( new ImoClef(dto) );
    }

    int get_clef_type()
    {
        string value = m_pParamToAnalyse->get_value();
        int type = ImoClef::k_G3;
        if (value == "G")
            type = ImoClef::k_G3;
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
            type = ImoClef::k_Percussion;
        else if (value == "C3")
            type = ImoClef::k_C3;
        else if (value == "C5")
            type = ImoClef::k_C5;
        else if (value == "F5")
            type = ImoClef::k_F5;
        else if (value == "G1")
            type = ImoClef::k_G1;
        else if (value == "8_G")
            type = ImoClef::k_8_G3;
        else if (value == "G_8")
            type = ImoClef::k_G3_8;
        else if (value == "8_F4")
            type = ImoClef::k_8_F4;
        else if (value == "F4_8")
            type = ImoClef::k_F4_8;
        else if (value == "15_G")
            type = ImoClef::k_15_G3;
        else if (value == "G_15")
            type = ImoClef::k_G3_15;
        else if (value == "15_F4")
            type = ImoClef::k_15_F4;
        else if (value == "F4_15")
            type = ImoClef::k_F4_15;
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown clef type '" + value + "'. Assumed 'G'.");
            LdpElement* value = m_pLdpFactory->new_value(k_label, "G");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
        }

        return type;
    }

};

//@-------------------------------------------------------------------------------------
//@ property
//@ <color> = (color <rgba>}
//@ <rgba> = label: { #rrggbb | #rrggbbaa }

class ColorAnalyser : public ElementAnalyser
{
public:
    ColorAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory)
        : ElementAnalyser(pAnalyser, reporter, pFactory) {}

    void do_analysis()
    {

        if (!get_optional(k_label) || !set_color())
        {
            error_and_remove_element("Missing or invalid color value. Must be #rrggbbaa. Color ignored.");
            return;
        }

        error_if_more_elements();
    }

protected:

    bool set_color()
    {
        ImoColorInfo* pColor = new ImoColorInfo();
        m_pAnalysedNode->set_imo(pColor);
        std::string value = get_string_value();
        pColor->get_from_string(value);
        return pColor->is_ok();
    }

};

//@-------------------------------------------------------------------------------------
//@ <content> = (content [<score>|<text>]*)

class ContentAnalyser : public ElementAnalyser
{
public:
    ContentAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoContent* pContent = new ImoContent();

        while (more_params_to_analyse())
        {
            if (! (analyse_optional(k_score, pContent)
                 || analyse_optional(k_text, pContent) ))
            {
                error_and_remove_invalid_param();
                move_to_next_param();
            }
        }

        error_if_more_elements();

        add_to_model(pContent);
    }
};

//@-------------------------------------------------------------------------------------
//@ ImoControl StaffObj
//@ <newSystem> = (newSystem}

class ControlAnalyser : public ElementAnalyser
{
public:
    ControlAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        add_to_model( new ImoControl() );
    }
};

//@-------------------------------------------------------------------------------------
//@ ImoFermata StaffObj
//@ <fermata> = (fermata <placement>[<componentOptions>*])
//@ <placement> = { above | below }

class FermataAnalyser : public ElementAnalyser
{
public:
    FermataAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                    ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoFermata dto;

        // <placement>
        if (get_mandatory(k_label))
            set_placement(dto);

        // [<componentOptions>*]
        analyse_component_options(dto);

        error_if_more_elements();

        add_to_model( new ImoFermata(dto) );
    }

    void set_placement(DtoFermata& dto)
    {
        string value = m_pParamToAnalyse->get_value();
        if (value == "above")
            dto.set_placement(ImoFermata::k_above);
        else if (value == "below")
            dto.set_placement(ImoFermata::k_below);
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown fermata placement '" + value + "'. Replaced by 'above'.");
            dto.set_placement(ImoFermata::k_above);
        }
    }

};

//@-------------------------------------------------------------------------------------
//@  <figuredBass> = (figuredBass <figuredBassSymbols>[<parenthesis>][<fbline>]
//@                               [<componentOptions>*] )
//@  <parenthesis> = (parenthesis { yes | no })  default: no
//@
//@  <figuredBassSymbols> = an string.
//@        It is formed by concatenation of individual strings for each interval.
//@        Each interval string is separated by a blank space from the previous one.
//@        And it can be enclosed in parenthesis.
//@        Each interval string is a combination of prefix, number and suffix,
//@        such as  "#3", "5/", "(3)", "2+" or "#".
//@        Valid values for prefix and suffix are:
//@            prefix = { + | - | # | b | = | x | bb | ## }
//@            suffix = { + | - | # | b | = | x | bb | ## | / | \ }
//@
//@  examples:
//@
//@    b6              (figuredBass "b6 b")
//@    b
//@
//@    6               (figuredBass "6 (3)")
//@   (3)
//@

class FiguredBassAnalyser : public ElementAnalyser
{
public:
    FiguredBassAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                        ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        //DtoFiguredBass dto;

        // <figuredBassSymbols> (string)
        if (get_mandatory(k_string))
        {}

        add_to_model( new ImoFiguredBass() );
    }
};

////    //get figured bass string and split it into components
////    wxString sData = GetNodeName(pNode->GetParameter(1));
////    lmFiguredBassData oFBData(sData);
////    if (oFBData.GetError() != _T(""))
////    {
////        AnalysisError(pNode, oFBData.GetError());
////        return (lmFiguredBass*)NULL;    //error
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
////        wxString sName = GetNodeName(pX);
////        if (sName == _T("parenthesis"))
////            ;   //TODO
////        else if (sName == _T("fbline"))     //start/end of figured bass line
////        {
////            if (nFBL > 1)
////                AnalysisError(pX, _T("[Element '%s'. More than two 'fbline'. Ignored."),
////                            sElmName.c_str() );
////            else
////                pFBLineInfo[nFBL++] = AnalyzeFBLine(pX, pVStaff);
////        }
////        else
////            AnalysisError(pX, _T("[Element '%s'. Invalid parameter '%s'. Ignored."),
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
////    lmFiguredBass* pFB = pVStaff->AddFiguredBass(&oFBData, nId);
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
//@ ImoGoBackFwd StaffObj
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
    GoBackFwdAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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
                    remove_this_element();
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
                    remove_this_element();
                    return;
                }
            }
            else
            {
                NoteTypeAndDots figdots = ldp_duration_to_components(duration);
                if (figdots.noteType == ImoNoteRest::k_unknown)
                {
                    report_msg(m_pParamToAnalyse->get_line_number(),
                        "Unknown duration '" + duration + "'. Element ignored.");
                    m_pAnalyser->erase_node(m_pParamToAnalyse);
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
                remove_this_element();
                return;
            }
            else
                dto.set_time_shift(rTime);
        }
        else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Unknown duration '" + m_pParamToAnalyse->get_name() + "'. Element ignored.");
            m_pAnalyser->erase_node(m_pParamToAnalyse);
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
    GroupAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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
            error_and_remove_element("Missing or invalid group symbol. Must be 'none', 'brace' or 'bracket'. Group removed.");
            delete pGrp;
            return;
        }

        // [<joinBarlines>]
        if (get_optional(k_joinBarlines))
            set_join_barlines(pGrp);

        // <instrument>+
        if (!more_params_to_analyse())
        {
            error_and_remove_element("Missing instruments in group!. Group removed.");
            delete pGrp;
            return;
        }
        else
        {
            while (more_params_to_analyse())
            {
                if (!analyse_optional(k_instrument, pGrp))
                {
                    error_and_remove_invalid_param();
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
//@ <infoMIDI> = (infoMIDI num_instr [num_channel])
//@ num_instr = integer: 1..256
//@ num_channel = integer: 1..16

class InfoMidiAnalyser : public ElementAnalyser
{
public:
    InfoMidiAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoMidiInfo dto;

        // num_instr
        if (!get_optional(k_number) || !set_instrument(dto))
        {
            error_and_remove_element("Missing or invalid MIDI instrument (1..256). MIDI info ignored.");
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
        int value = get_integer_number(0);
        if (value < 1 || value > 256)
            return false;   //error
        dto.set_instrument(value-1);
        return true;
    }

    bool set_channel(ImoMidiInfo& dto)
    {
        int value = get_integer_number(0);
        if (value < 1 || value > 16)
            return false;   //error
        dto.set_channel(value-1);
        return true;
    }

};

//@-------------------------------------------------------------------------------------
//@ <instrument> = (instrument [<instrName>][<instrAbbrev>][<infoMIDI>]
//@                            [<staves>] <musicData> )
//@ <instrName> = <textString>
//@ <instrAbbrev> = <textString>

class InstrumentAnalyser : public ElementAnalyser
{
public:
    InstrumentAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                       ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        m_pAnalyser->clear_pending_ties();
        m_pAnalyser->clear_pending_beams();
        m_pAnalyser->clear_pending_tuplets();

        ImoInstrument* pInstrument = new ImoInstrument();

        // [<name>]
        analyse_optional(k_name, pInstrument);

        // [<abbrev>]
        analyse_optional(k_abbrev, pInstrument);

        // [<infoMIDI>]
        analyse_optional(k_infoMIDI, pInstrument);

        // [<staves>]
        if (get_optional(k_staves))
            set_staves(pInstrument);

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
            LdpElement* value = m_pLdpFactory->new_value(k_number, "1");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            pInstrument->set_num_staves(1);
        }
        else
        {
            //TODO: check that maximum number of supported staves is not reached
            //sNumStaves.ToLong(&nNumStaves);
            //if (nNumStaves > lmMAX_STAFF)
            //{
            //    AnalysisError(pX, _T("Program limit reached: the number of staves per instrument must not be greater than %d. Please inform Lomse developers."),
            //                  lmMAX_STAFF);
            //    nNumStaves = lmMAX_STAFF;
            //}
            pInstrument->set_num_staves(nStaves);
        }
    }

};

//@-------------------------------------------------------------------------------------
//@ ImoKeySignature StaffObj
//@ <key> = (key <type>[<staffobjOptions>*] )
//@ <type> = label: { C | G | D | A | E | B | F+ | C+ | C- | G- | D- | A- |
//@                   E- | B- | F | a | e | b | f+ | c+ | g+ | d+ | a+ | a- |
//@                   e- | b- | f | c | g | d }

class KeySignatureAnalyser : public ElementAnalyser
{
public:
    KeySignatureAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoKeySignature dto(ImoKeySignature::C);

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
        int type = ImoKeySignature::C;
        if (value == "C")
            type = ImoKeySignature::C;
        else if (value == "G")
            type = ImoKeySignature::G;
        else if (value == "D")
            type = ImoKeySignature::D;
        else if (value == "A")
            type = ImoKeySignature::A;
        else if (value == "E")
            type = ImoKeySignature::E;
        else if (value == "B")
            type = ImoKeySignature::B;
        else if (value == "F+")
            type = ImoKeySignature::Fs;
        else if (value == "C+")
            type = ImoKeySignature::Cs;
        else if (value == "C-")
            type = ImoKeySignature::Cf;
        else if (value == "G-")
            type = ImoKeySignature::Gf;
        else if (value == "D-")
            type = ImoKeySignature::Df;
        else if (value == "A-")
            type = ImoKeySignature::Af;
        else if (value == "E-")
            type = ImoKeySignature::Ef;
        else if (value == "B-")
            type = ImoKeySignature::Bf;
        else if (value == "F")
            type = ImoKeySignature::F;
        else if (value == "a")
            type = ImoKeySignature::a;
        else if (value == "e")
            type = ImoKeySignature::e;
        else if (value == "b")
            type = ImoKeySignature::b;
        else if (value == "f+")
            type = ImoKeySignature::fs;
        else if (value == "c+")
            type = ImoKeySignature::cs;
        else if (value == "g+")
            type = ImoKeySignature::gs;
        else if (value == "d+")
            type = ImoKeySignature::ds;
        else if (value == "a+")
            type = ImoKeySignature::as;
        else if (value == "a-")
            type = ImoKeySignature::af;
        else if (value == "e-")
            type = ImoKeySignature::ef;
        else if (value == "b-")
            type = ImoKeySignature::bf;
        else if (value == "f")
            type = ImoKeySignature::f;
        else if (value == "c")
            type = ImoKeySignature::c;
        else if (value == "g")
            type = ImoKeySignature::g;
        else if (value == "d")
            type = ImoKeySignature::d;
       else
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                    "Unknown key '" + value + "'. Assumed 'C'.");
            LdpElement* value = m_pLdpFactory->new_value(k_label, "C");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
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
    LanguageAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory)
        : ElementAnalyser(pAnalyser, reporter, pFactory) {}

    void do_analysis()
    {
        remove_this_element();
    }
};

//@-------------------------------------------------------------------------------------
//@ <lenmusdoc> = (lenmusdoc <vers> <content>)

class LenmusdocAnalyser : public ElementAnalyser
{
public:
    LenmusdocAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory)
        : ElementAnalyser(pAnalyser, reporter, pFactory) {}

    void do_analysis()
    {
        ImoDocument* pDoc = NULL;

        // <vers>
        if (get_mandatory(k_vers))
        {
            string version = get_version();
            pDoc = new ImoDocument(version);
        }

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
    MetronomeAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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
                dto.set_ticks_per_minute( get_integer_number(60) );
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
            dto.set_ticks_per_minute( get_integer_number(60) );
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
                error_and_remove_invalid_param();
        }

        // [<componentOptions>*]
        analyse_component_options(dto);

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
    MusicDataAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                      ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoMusicData* pMD = new ImoMusicData();

        // [{<xxxx>|<yyyy>|<zzzz>}*]    alternatives: zero or more
        while (more_params_to_analyse())
        {
            if (! (analyse_optional(k_note, pMD)
                   || analyse_optional(k_rest, pMD)
                   || analyse_optional(k_barline, pMD)
                   || analyse_optional(k_chord, pMD)
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
                error_and_remove_invalid_param();
                move_to_next_param();
            }
        }

        add_to_model(pMD);
    }

};

//@-------------------------------------------------------------------------------------
//@ ImoNote, ImoRest StaffObj
//@ <note> = (n <pitch><duration> [<noteOptions>*][<noteRestOptions>*]
//@             [<componentOptions>*][<attachments>*])
//@ <rest> = (r <duration> [<noteRestOptions>*][<componentOptions>*][<attachments>*])
//@ <noteOptions> = { <tie> | <stem> }
//@ <noteRestOptions> = { <beam> | <tuplet> | <voice> | <staffNum> | <fermata> }
//@ <pitch> = label
//@ <duration> = label
//@ <stem> = (stem { up | down })
//@ <voice> = (voice num)
//@ <staffNum> = (p num)

class NoteRestAnalyser : public ElementAnalyser
{
protected:
    ImoTieDto* m_pTieInfo;
    ImoTupletDto* m_pTupletInfo;
    ImoBeamInfo* m_pBeamInfo;
    ImoFermata* m_pFermata;
    std::string m_srcOldBeam;
    std::string m_srcOldTuplet;

public:
    NoteRestAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                     ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor)
        , m_pTieInfo(NULL)
        , m_pTupletInfo(NULL)
        , m_pBeamInfo(NULL)
        , m_pFermata(NULL)
        , m_srcOldBeam("")
        , m_srcOldTuplet("") {}

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

        if (!fIsRest)
            dtoNote.set_in_chord(fInChord);
        //TODO: check if note in chord has the same duration than base note
      //  if (fInChord && m_pLastNoteRest && m_pLastNoteRest->IsNote()
      //      && !IsEqualTime(m_pLastNoteRest->GetDuration(), rDuration) )
      //  {
      //      report_msg("Error: note in chord has different duration than base note. Duration changed."));
		    //rDuration = m_pLastNoteRest->GetDuration();
      //      nNoteType = m_pLastNoteRest->GetNoteType();
      //      nDots = m_pLastNoteRest->GetNumDots();
      //  }

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
                error_and_remove_invalid_param();
        }
        pDto->set_staff( m_pAnalyser->get_current_staff() );
        pDto->set_voice( m_pAnalyser->get_current_voice() );


        if (!fIsRest)
        {
            // [<noteOptions>*] = [{ <tie> | <stem> }*]
            while (more_params_to_analyse())
            {
                if (get_optional(k_tie))
                    m_pTieInfo = dynamic_cast<ImoTieDto*>( proceed(k_tie, NULL) );
                else if (get_optional(k_stem))
                    set_stem(dtoNote);
                else
                    break;
            }
        }

        // [<noteRestOptions>*]
        analyse_note_rest_options(pDto);

        // [<componentOptions>*]
        analyse_component_options(*pDto);

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
            pNR->attach(m_pFermata);

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
                m_pBeamInfo = dynamic_cast<ImoBeamInfo*>( pImo );
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
            LdpElement* value = m_pLdpFactory->new_value(k_pitch, "c4");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            dto.set_pitch(ImoNote::C, 4, ImoNote::k_no_accidentals);
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
                + "'. Removed.");
            m_pAnalyser->erase_node(m_pParamToAnalyse);
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
            LdpElement* value = m_pLdpFactory->new_value(k_label, "v1");
            m_pAnalyser->replace_node(m_pParamToAnalyse, value);
            m_pAnalyser->set_current_voice(1);
        }
        else
            m_pAnalyser->set_current_voice(nVoice);
    }

    void set_stem(DtoNote& dto)
    {
        LdpElement* pStem = m_pParamToAnalyse;
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        string value = get_string_value();
        if (value == "up")
            dto.set_stem_direction(ImoNote::k_up);
        else if (value == "down")
            dto.set_stem_direction(ImoNote::k_down);
        else
        {
            dto.set_stem_direction(ImoNote::k_default);
            report_msg(m_pParamToAnalyse->get_line_number(),
                            "Invalid value '" + value
                            + "' for stem type. Default stem asigned.");
            m_pAnalyser->erase_node(pStem);
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
        ImoBeamInfo* pInfo = new ImoBeamInfo();
        pInfo->set_note_rest(pNR);
        m_pAnalyser->add_old_beam(pInfo);
    }

    void error_note_longer_than_eighth()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
            "Requesting beaming a note longer than eighth. Beam ignored.");
        m_pAnalyser->erase_node(m_pParamToAnalyse);
    }

    void error_beam_already_open()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
            "Requesting to start a beam (g+) but there is already an open beam. Beam ignored.");
        m_pAnalyser->erase_node(m_pParamToAnalyse);
    }

    void error_no_beam_open()
    {
        report_msg(m_pParamToAnalyse->get_line_number(),
            "Requesting to end a beam (g-) but there is no matching g+. Beam ignored.");
        m_pAnalyser->erase_node(m_pParamToAnalyse);
    }

    void end_g_beam(ImoNoteRest* pNR)
    {
        if (!m_pAnalyser->is_old_beam_open())
        {
            error_no_beam_open();
        }
        else
        {
            ImoBeamInfo* pInfo = new ImoBeamInfo();
            pInfo->set_note_rest(pNR);
            m_pAnalyser->close_old_beam(pInfo);
        }
    }

    int get_beaming_level(int nNoteType)
    {
        switch(nNoteType) {
            case ImoNoteRest::k_eighth:
                return 0;
            case ImoNoteRest::k_16th:
                return 1;
            case ImoNoteRest::k_32th:
                return 2;
            case ImoNoteRest::k_64th:
                return 3;
            case ImoNoteRest::k_128th:
                return 4;
            case ImoNoteRest::k_256th:
                return 5;
            default:
                return -1; //Error: Requesting beaming a note longer than eight
        }
    }

    void add_to_current_tuplet(ImoNoteRest* pNR)
    {
        ImoTupletDto* pInfo = new ImoTupletDto();
        pInfo->set_note_rest(pNR);
        m_pAnalyser->add_tuplet_info(pInfo);
    }

    void set_old_tuplet(ImoNoteRest* pNR)
    {
        string value = m_srcOldTuplet;
        bool fError = false;
        char numActual;
        char numNormal = '0';
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
                    numActual = value[1];
            }
            else if (value.length() == 4 && value[2] == '/')
            {
                numActual = value[1];
                numNormal = value[3];
            }
            else
                fError = true;
        }
        else
            fError = true;

        locale loc;
        if (!fError && (!isdigit(numActual,loc) || !isdigit(numNormal,loc)))
            fError = true;

        if (fError)
        {
            report_msg(m_pParamToAnalyse->get_line_number(),
                "Invalid parameter '" + m_pParamToAnalyse->get_value()
                + "'. Removed.");
            m_pAnalyser->erase_node(m_pParamToAnalyse);
        }
        else
        {
            if (m_pAnalyser->is_tuplet_open())
            {
                report_msg(m_pParamToAnalyse->get_line_number(),
                    "Requesting to start a tuplet but there is already an open tuplet. Tuplet ignored.");
                m_pAnalyser->erase_node(m_pParamToAnalyse);

                add_to_current_tuplet(pNR);
            }
            else
            {
                int actual;
                int normal;
                std::string sa(&numActual);
                std::string sn(&numNormal);
                stringstream(sa) >> actual;
                stringstream(sn) >> normal;
                start_old_tuplet(pNR, actual, normal);
            }
        }
    }

    void end_old_tuplet(ImoNoteRest* pNR)
    {
        ImoTupletDto* pInfo = new ImoTupletDto();
        pInfo->set_note_rest(pNR);
        pInfo->set_start_of_tuplet(false);
        m_pAnalyser->add_tuplet_info(pInfo);
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
        m_pAnalyser->add_tuplet_info(pInfo);
    }

    void set_voice_element(DtoNoteRest* pDto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int voice = get_integer_number( m_pAnalyser->get_current_voice() );
        m_pAnalyser->set_current_voice(voice);
        pDto->set_voice(voice);
    }

    void set_staff_num_element(DtoNoteRest* pDto)
    {
        m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
        int curStaff = m_pAnalyser->get_current_staff() + 1;
        int staff = get_integer_number(curStaff) - 1;
        m_pAnalyser->set_current_staff(staff);
        pDto->set_staff(staff);
    }

    void add_tie_info(ImoNote* pNote)
    {
        if (m_pTieInfo)
        {
            m_pTieInfo->set_note(pNote);
            if (m_pTieInfo->is_start())
                m_pAnalyser->start_tie(m_pTieInfo);
            else
                m_pAnalyser->end_tie(m_pTieInfo);
        }
    }

    void add_tuplet_info(ImoNoteRest* pNR)
    {
        if (m_pTupletInfo)
        {
            m_pTupletInfo->set_note_rest(pNR);
            m_pAnalyser->add_tuplet_info(m_pTupletInfo);
        }
    }

    void add_beam_info(ImoNoteRest* pNR)
    {
        if (m_pBeamInfo)
        {
            m_pBeamInfo->set_note_rest(pNR);
            m_pAnalyser->add_beam_info(m_pBeamInfo);
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
    OptAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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

                remove_this_element();
            }
        }
        else
            error_and_remove_element("Missing value for option '"
                                     + name + "'. Option ignored.");
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
            pOpt->set_long_value( get_long_number() );
            pOpt->set_type(ImoOptionInfo::k_number_long);
            return true;    //ok
        }
        return false;   //error
    }

    bool set_float_value(ImoOptionInfo* pOpt)
    {
        if (is_float_value())
        {
            pOpt->set_float_value( get_float_number() );
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
//@ <score> = (score <vers>[<language>][<undoData>][<creationMode>][<defineStyle>*]
//@                  [<title>*][<pageLayout>*][<systemLayout>*][<cursor>][<option>*]
//@                  {<instrument> | <group>}* )

class ScoreAnalyser : public ElementAnalyser
{
public:
    ScoreAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                  ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoScore* pScore = new ImoScore();

        // <vers>
        if (get_mandatory(k_vers))
            pScore->set_version( get_version() );

        // [<language>]
        analyse_optional(k_language);

        // [<undoData>]
        analyse_optional(k_undoData);

        // [<creationMode>]
        analyse_optional(k_creationMode);

        // [<defineStyle>*]
        while (analyse_optional(k_defineStyle));

        // [<title>*]
        while (analyse_optional(k_title));

        // [<pageLayout>*]
        while (analyse_optional(k_pageLayout));

        // [<systemLayout>*]
        while (analyse_optional(k_systemLayout, pScore));

        // [<cursor>]
        analyse_optional(k_cursor);

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
                    error_and_remove_invalid_param();
                    move_to_next_param();
                }
            }
        }

        error_if_more_elements();

        add_to_model(pScore);
    }

protected:

    string get_version()
    {
        return m_pParamToAnalyse->get_parameter(1)->get_value();
    }

};

//@-------------------------------------------------------------------------------------
//@ ImoSpacer StaffObj
//@ <spacer> = (spacer <width>[<staffobjOptions>*][<attachments>*])     width in Tenths

class SpacerAnalyser : public ElementAnalyser
{
public:
    SpacerAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoSpacer dto;

        // <width>
        if (get_optional(k_number))
        {
            dto.set_width( get_float_number() );
        }
        else
        {
            error_and_remove_element("Missing width for spacer. Spacer removed.");
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
//@ <systemLayout> = (systemLayout {first | other} <systemMargins>)

class SystemLayoutAnalyser : public ElementAnalyser
{
public:
    SystemLayoutAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                         ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

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
                LdpElement* value = m_pLdpFactory->new_value(k_label, "first");
                m_pAnalyser->replace_node(m_pParamToAnalyse, value);
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
    SystemMarginsAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoSystemInfo dto;
        ImoSystemInfo* pDto;
        if (m_pAnchor && m_pAnchor->is_system_info())
            pDto = dynamic_cast<ImoSystemInfo*>(m_pAnchor);
        else
            pDto = &dto;

        if (get_mandatory(k_number))
            pDto->set_left_margin(get_float_number());

        if (get_mandatory(k_number))
            pDto->set_right_margin(get_float_number());

        if (get_mandatory(k_number))
            pDto->set_system_distance(get_float_number());

        if (get_mandatory(k_number))
            pDto->set_top_system_distance(get_float_number());

        error_if_more_elements();
    }

};

//@-------------------------------------------------------------------------------------
//@ <textString> = (<textTag> string [<location>][{<font> | <style>}][<alingment>])
//@ <textTag> = { name | abbrev | text }
//@ <style> = (style <name>)
//@
//@ Notes:
//@ - <font> is obsolete (since 1.6) and maintained just for compatibility

class TextStringAnalyser : public ElementAnalyser
{
public:
    TextStringAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                 ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        // <string>
        if (get_mandatory(k_string))
        {
            ImoTextString* pText = new ImoTextString(get_string_value());

            // [<location>]
            //TODO

            // [{<font> | <style>}]
            //TODO

            // [<alingment>]
            //TODO

            add_to_model(pText);
        }
    }

protected:

};
////bool lmLDPParser::AnalyzeText(lmLDPNode* pNode, lmVStaff* pVStaff, lmStaffObj* pTarget)
////{
////    //returns true if error; in this case nothing is added to the VStaff
////    // <text> = (text string <location>[<font><alingment>])
////
////    wxASSERT(GetNodeName(pNode) == _T("text"));
////    long nId = GetNodeID(pNode);
////
////    //check that at least two parameters (location and text string) are specified
////    if(GetNodeNumParms(pNode) < 2) {
////        AnalysisError(
////            pNode,
////            _T("Element '%s' has less parameters than the minimum required. Element ignored."),
////            _T("text") );
////        return true;
////    }
////
////    wxString sText;
////    wxString sStyle;
////    lmEHAlign nAlign = lmHALIGN_LEFT;     //TODO user options instead of fixed values
////    lmFontInfo tFont = {m_sTextFontName, m_nTextFontSize, m_nTextStyle, m_nTextWeight};
////    lmLocation tPos;
////    tPos.xUnits = lmTENTHS;
////    tPos.yUnits = lmTENTHS;
////    tPos.x = 0.0f;
////    tPos.y = 0.0f;
////
////    if (AnalyzeTextString(pNode, &sText, &sStyle, &nAlign, &tPos, &tFont))
////        return true;
////
////    //no error:
////    //save font values as new default for texts
////    m_sTextFontName = tFont.sFontName;
////    m_nTextFontSize = tFont.nFontSize;
////    m_nTextStyle = tFont.nFontStyle;
////    m_nTextWeight = tFont.nFontWeight;
////
////    //create the text
////    lmTextStyle* pStyle = (lmTextStyle*)NULL;
////    if (sStyle != _T(""))
////    {
////        pStyle = pVStaff->GetScore()->GetStyleInfo(sStyle);
////        if (!pStyle)
////            AnalysisError(pNode, _T("Style '%s' is not defined. Default style will be used."),
////                           sStyle.c_str());
////    }
////
////    if (!pStyle)
////        pStyle = pVStaff->GetScore()->GetStyleName(tFont);
////
////    if (!pTarget)
////        pTarget = pVStaff->AddAnchorObj();  //AWARE: generating a text element without anchor
////                                            //object is no longer possible since v1.6. Therefore,
////                                            //for undo/redo all text elements will have anchor
////                                            //with ID correctly saved/restored
////
////    lmTextItem* pText = pVStaff->AddText(sText, nAlign, pStyle, pTarget, nId);
////	pText->SetUserLocation(tPos);
////
////    return false;
////}


//@-------------------------------------------------------------------------------------
//@ <tie> = (tie num <tieType>[<bezier>][color] )   ;num = tie number. integer
//@ <tieType> = { start | stop }

class TieAnalyser : public ElementAnalyser
{
public:
    TieAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoTieDto* pInfo = new ImoTieDto();

        // num
        if (get_mandatory(k_number))
            pInfo->set_tie_number( get_integer_number(0) );

        // <tieType> (label)
        if (!get_mandatory(k_label) || !set_tie_type(pInfo))
        {
            error_and_remove_element("Missing or invalid tie type. Tie ignored.");
            delete pInfo;
            return;
        }

        // [<bezier>]
        analyse_optional(k_bezier, pInfo);

        // [<color>]
        //TODO

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
//@ ImoTimeSignature StaffObj
//@ <timeSignature> = (time <beats><beatType>[<visible>][<location>])
//@ <beats> = <num>
//@ <beatType> = <num>

class TimeSignatureAnalyser : public ElementAnalyser
{
public:
    TimeSignatureAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                          ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        DtoTimeSignature dto;

        // <beats> (num)
        if (get_mandatory(k_number))
            dto.set_beats( get_integer_number(2) );

        // <beatType> (num)
        if (get_mandatory(k_number))
            dto.set_beat_type( get_integer_number(4) );

        // [<visible>][<location>]
        analyse_staffobjs_options(dto);

        add_to_model( new ImoTimeSignature(dto) );
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
//@ <tuplet> = (t num { - | + <actualNotes>[<normalNotes>][<tupletOptions>] } )
//@ <actualNotes> = num
//@ <normalNotes> = num
//@ <tupletOptions> = noBracket
//@    future options: squaredBracket | curvedBracket |
//@                    numNone | numActual | numBoth
//@

class TupletAnalyser : public ElementAnalyser
{
public:
    TupletAnalyser(Analyser* pAnalyser, ostream& reporter, LdpFactory* pFactory,
                   ImoObj* pAnchor)
        : ElementAnalyser(pAnalyser, reporter, pFactory, pAnchor) {}

    void do_analysis()
    {
        ImoTupletDto* pInfo = new ImoTupletDto();
        set_default_values(pInfo);

        // { + | - }
        if (!get_mandatory(k_label) || !set_tuplet_type(pInfo))
        {
            error_and_remove_element("Missing or invalid tuplet type. Tuplet ignored.");
            delete pInfo;
            return;
        }

        if (pInfo->is_start_of_tuplet())
        {
            // <actualNotes>
            if (!get_mandatory(k_number) || !set_actual_notes(pInfo))
            {
                error_and_remove_element("Tuplet: missing or invalid actual notes number. Tuplet ignored.");
                delete pInfo;
                return;
            }

            // [<normalNotes>]
            if (get_optional(k_number))
                set_normal_notes(pInfo);
            if (pInfo->get_normal_number() == 0)
            {
                error_and_remove_element("Tuplet: Missing or invalid normal notes number. Tuplet ignored.");
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
        pInfo->set_show_number( m_pAnalyser->get_current_show_tuplet_number() );
    }

    bool set_tuplet_type(ImoTupletDto* pInfo)
    {
        const std::string& value = m_pParamToAnalyse->get_value();
        if (value == "+")
            pInfo->set_start_of_tuplet(true);
        else if (value == "-")
            pInfo->set_start_of_tuplet(false);
        else
            return false;   //error
        return true;    //ok
    }

    bool set_actual_notes(ImoTupletDto* pInfo)
    {
        int actual = get_integer_number(0);
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
        int normal = get_integer_number(0);
        pInfo->set_normal_number(normal);
    }

    void analyse_tuplet_options(ImoTupletDto* pInfo)
    {
        bool fShowBracket = m_pAnalyser->get_current_show_tuplet_bracket();
        while( more_params_to_analyse() )
        {
            m_pParamToAnalyse = get_param_to_analyse();
            ELdpElement type = m_pParamToAnalyse->get_type();
            if (type == k_label)
            {
                const std::string& value = m_pParamToAnalyse->get_value();
                if (value == "noBracket")
                    fShowBracket = false;
                else
                    error_and_remove_invalid_param();
            }
            else
                error_and_remove_invalid_param();

            move_to_next_param();
        }

        pInfo->set_show_bracket(fShowBracket);
        m_pAnalyser->set_current_show_tuplet_bracket(fShowBracket);
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
                "Element '" + name + "' unknown or not possible here. Removed.");
            m_pAnalyser->erase_node(m_pParamToAnalyse);
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

void ElementAnalyser::error_and_remove_invalid_param()
{
    ELdpElement type = m_pParamToAnalyse->get_type();
    string name = m_pLdpFactory->get_name(type);
    if (name == "label")
        name += ":" + m_pParamToAnalyse->get_value();
    report_msg(m_pParamToAnalyse->get_line_number(),
        "Element '" + name + "' unknown or not possible here. Removed.");
    m_pAnalyser->erase_node(m_pParamToAnalyse);
}

void ElementAnalyser::error_and_remove_element(const string& msg)
{
    report_msg(m_pAnalysedNode->get_line_number(), msg);
    remove_this_element();
}

void ElementAnalyser::remove_this_element()
{
    m_pAnalyser->erase_node(m_pAnalysedNode);
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
                + name + "' have been removed.");
        while (more_params_to_analyse())
        {
            m_pParamToAnalyse = get_param_to_analyse();
            move_to_next_param();
            m_pAnalyser->erase_node(m_pParamToAnalyse);
        }
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
            error_and_remove_invalid_param();
    }

    //set staff: either found value or inherited one
    dto.set_staff( m_pAnalyser->get_current_staff() );

    analyse_component_options(dto);
}

void ElementAnalyser::analyse_component_options(DtoComponentObj& dto)
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
                ImoObj* pImo = m_pAnalyser->analyse_node(m_pParamToAnalyse, NULL);
                ImoColorInfo* pColor = dynamic_cast<ImoColorInfo*>( pImo );
                if (pColor)
                {
                    dto.set_color( pColor->get_color() );
                    delete pColor;
                }
                break;
            }
            case k_dx:
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                dto.set_user_location_x( get_float_number(0.0f) );
                break;
            }
            case k_dy:
            {
                m_pParamToAnalyse = m_pParamToAnalyse->get_parameter(1);
                dto.set_user_location_y( get_float_number(0.0f) );
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
    Linker linker;
    ImoObj* pObj = linker.add_child_to_model(m_pAnchor, pImo, ldpNodeType);
    m_pAnalysedNode->set_imo(pObj);
}




//--------------------------------------------------------------------------------
// Analyser implementation
//--------------------------------------------------------------------------------

Analyser::Analyser(ostream& reporter, LdpFactory* pFactory)
    : m_reporter(reporter)
    , m_pLdpFactory(pFactory)
    , m_pTiesBuilder(NULL)
    , m_pBeamsBuilder(NULL)
    , m_pOldBeamsBuilder(NULL)
    , m_pTupletsBuilder(NULL)
    , m_pBasicModel(NULL)
    , m_pTree(NULL)
    , m_fShowTupletBracket(true)
    , m_fShowTupletNumber(true)
{
}

Analyser::~Analyser()
{
    if (m_pTiesBuilder)
        delete m_pTiesBuilder;
    if (m_pBeamsBuilder)
        delete m_pBeamsBuilder;
    if (m_pOldBeamsBuilder)
        delete m_pOldBeamsBuilder;
    if (m_pTupletsBuilder)
        delete m_pTupletsBuilder;
}

//void Analyser::add_beam(ImoBeam* pBeam)
//{
//    m_pBasicModel->add_beam(pBeam);
//}
//
//void Analyser::add_tuplet(ImoTuplet* pTuplet)
//{
//    m_pBasicModel->add_tuplet(pTuplet);
//}
//
//void Analyser::add_tie(ImoTie* pTie)
//{
//    m_pBasicModel->add_tie(pTie);
//}

BasicModel* Analyser::analyse_tree(LdpTree* tree)
{
    m_pBasicModel = new BasicModel();
    if (m_pTiesBuilder)
        delete m_pTiesBuilder;
    if (m_pBeamsBuilder)
        delete m_pBeamsBuilder;
    if (m_pOldBeamsBuilder)
        delete m_pOldBeamsBuilder;
    if (m_pTupletsBuilder)
        delete m_pTupletsBuilder;
    m_pTiesBuilder = new TiesBuilder(m_reporter, m_pBasicModel, this);
    m_pBeamsBuilder = new BeamsBuilder(m_reporter, m_pBasicModel, this);
    m_pOldBeamsBuilder = new OldBeamsBuilder(m_reporter, m_pBasicModel, this);
    m_pTupletsBuilder = new TupletsBuilder(m_reporter, m_pBasicModel, this);

    m_pTree = tree;
    m_curStaff = 0;
    m_curVoice = 1;
    analyse_node(tree->get_root());
    m_pTiesBuilder->clear_pending_ties();

    LdpElement* pRoot = tree->get_root();
    if (pRoot)
        m_pBasicModel->set_root( pRoot->get_imo() );

    BasicModel* pBM = m_pBasicModel;
    m_pBasicModel = NULL;
    return pBM;
}

void Analyser::analyse_node(LdpTree::iterator itNode)
{
    analyse_node(*itNode);
}

ImoObj* Analyser::analyse_node(LdpElement* pNode, ImoObj* pAnchor)
{
    ElementAnalyser* a = new_analyser( pNode->get_type(), pAnchor );
    a->analyse_node(pNode);
    delete a;
    return pNode->get_imo();
}

void Analyser::erase_node(LdpElement* pNode)
{
    if (pNode)
    {
        ImoObj* pImo = pNode->get_imo();
        if (pImo) delete pImo;
        LdpTree::iterator it(pNode);
        m_pTree->erase(it);
        delete pNode;
    }
}

void Analyser::replace_node(LdpElement* pOldNode, LdpElement* pNewNode)
{
    ImoObj* pImo = pOldNode->get_imo();
    if (pImo) delete pImo;
    LdpTree::iterator it(pOldNode);
    m_pTree->replace_node(it, pNewNode);
    delete pOldNode;
}

void Analyser::remove_tie_element(ImoTieDto* pInfo)
{
//    LdpElement* pNode = pInfo->get_tie_element();
//    if (pNode)      //in tests node could no exist
//        erase_node(pNode);      //also deletes pInfo
//    else
//    //    pInfo->clear_tie();     //prevent ImoTieDto to delete pTie
//    //}
        delete pInfo;
}

void Analyser::remove_old_tie_element(LdpElement* pOldTieParam)
{
    erase_node(pOldTieParam);
}

ElementAnalyser* Analyser::new_analyser(ELdpElement type, ImoObj* pAnchor)
{
    //Factory method to create analysers

    switch (type)
    {
        case k_abbrev:          return new TextStringAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_above:
        case k_barline:         return new BarlineAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_below:
        case k_beam:            return new BeamAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_bezier:          return new BezierAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_bold:
//        case k_bold_italic:
//        case k_brace:
//        case k_bracket:
//        case k_center:
        case k_chord:           return new ChordAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_clef:            return new ClefAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_content:         return new ContentAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_creationMode:
        case k_color:           return new ColorAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_cursor:
//        case k_defineStyle:   return new  XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_end:
        case k_fermata:         return new FermataAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_figuredBass:     return new FiguredBassAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_font:
        case k_goBack:          return new GoBackFwdAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_goFwd:           return new GoBackFwdAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_graphic:   return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_group:           return new GroupAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_hasWidth:
        case k_infoMIDI:        return new InfoMidiAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_instrument:      return new InstrumentAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_italic:
//        case k_joinBarlines:
        case k_key_signature:             return new KeySignatureAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_landscape:
        case k_language:        return new LanguageAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_left:
        case k_lenmusdoc:       return new LenmusdocAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_line:  return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
        case k_metronome:       return new MetronomeAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_musicData:       return new MusicDataAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_name:            return new TextStringAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_newSystem:       return new ControlAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_no:
//        case k_normal:
        case k_note:            return new NoteRestAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_opt:             return new OptAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_pageLayout:    return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_pageMargins:   return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_pageSize:
        case k_rest:            return new NoteRestAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_right: return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
        case k_score:           return new ScoreAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_spacer:          return new SpacerAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_split: return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_staff: return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_start: return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_style:
//        case k_symbol:    return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
        case k_systemLayout:    return new SystemLayoutAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_systemMargins:   return new SystemMarginsAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_text:            return new TextStringAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_textbox:       return new TextBoxAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_time_signature:  return new TimeSignatureAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
        case k_tie:             return new TieAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_title:         return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
        case k_tuplet:          return new TupletAnalyser(this, m_reporter, m_pLdpFactory, pAnchor);
//        case k_undoData:  return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);
//        case k_yes:   return new XxxxxxxAnalyser(this, m_reporter, m_pLdpFactory);

        default:
            return new NullAnalyser(this, m_reporter, m_pLdpFactory);
    }
}


//-------------------------------------------------------------------------------
// TiesBuilder implementation
//-------------------------------------------------------------------------------

TiesBuilder::TiesBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pBasicModel(pBasicModel)
    , m_pStartNoteTieOld(NULL)
{
}

TiesBuilder::~TiesBuilder()
{
    clear_pending_ties();
}

void TiesBuilder::start_tie(ImoTieDto* pNewInfo)
{
    ImoTieDto* pExistingInfo = find_matching_tie_info(pNewInfo);
    if (pExistingInfo)
    {
        error_duplicated_tie(pExistingInfo, pNewInfo);
        m_pAnalyser->remove_tie_element(pNewInfo);
    }
    else
    {
        m_pendingTies.push_back(pNewInfo);
    }
}

void TiesBuilder::end_tie(ImoTieDto* pEndInfo)
{
    ImoTieDto* pStartInfo = find_matching_tie_info(pEndInfo);
    if (pStartInfo)
    {
        if (notes_can_be_tied(pStartInfo, pEndInfo))
        {
            tie_notes(pStartInfo, pEndInfo);
            remove_from_pending(pStartInfo);
            delete pStartInfo;
            delete pEndInfo;
        }
        else
        {
            error_notes_can_not_be_tied(pEndInfo);
            remove_from_pending(pStartInfo);
            m_pAnalyser->remove_tie_element(pStartInfo);
            m_pAnalyser->remove_tie_element(pEndInfo);
        }
    }
    else
    {
        error_no_start_tie(pEndInfo);
        m_pAnalyser->remove_tie_element(pEndInfo);
    }
}

ImoTieDto* TiesBuilder::find_matching_tie_info(ImoTieDto* pEndInfo)
{
    std::list<ImoTieDto*>::iterator it;
    for(it=m_pendingTies.begin(); it != m_pendingTies.end(); ++it)
    {
         if ((*it)->get_tie_number() == pEndInfo->get_tie_number())
             return *it;
    }
    return NULL;
}

bool TiesBuilder::notes_can_be_tied(ImoTieDto* pStartInfo, ImoTieDto* pEndInfo)
{
    ImoNote* pStartNote = pStartInfo->get_note();
    ImoNote* pEndNote = pEndInfo->get_note();
    return notes_can_be_tied(pStartNote, pEndNote);
}

bool TiesBuilder::notes_can_be_tied(ImoNote* pStartNote, ImoNote* pEndNote)
{
    return (pStartNote->get_voice() == pEndNote->get_voice())
            && (pStartNote->get_staff() == pEndNote->get_staff())
            && (pStartNote->get_accidentals() == pEndNote->get_accidentals())
            && (pStartNote->get_step() == pEndNote->get_step())
            && (pStartNote->get_octave() == pEndNote->get_octave()) ;
}

void TiesBuilder::tie_notes(ImoTieDto* pStartInfo, ImoTieDto* pEndInfo)
{
    ImoNote* pStartNote = pStartInfo->get_note();
    ImoNote* pEndNote = pEndInfo->get_note();

    ImoTie* pTie = new ImoTie();
    pTie->set_tie_number( pStartInfo->get_tie_number() );
    pTie->set_start_note( pStartNote );
    pTie->set_end_note( pEndNote );
    pTie->set_start_bezier( pStartInfo->get_bezier() );
    pTie->set_stop_bezier( pEndInfo->get_bezier() );

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);

    //pStartNote->set_tied_next(true);
    //pEndNote->set_tied_prev(true);
    m_pBasicModel->add_tie(pTie);
}

void TiesBuilder::error_duplicated_tie(ImoTieDto* pExistingInfo, ImoTieDto* pNewInfo)
{
    m_reporter << "Line " << pNewInfo->get_line_number()
               << ". This tie has the same number than that defined in line "
               << pExistingInfo->get_line_number()
               << ". This tie will be ignored." << endl;
}

void TiesBuilder::error_notes_can_not_be_tied(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". Requesting to tie notes of different voice or pitch. Tie number "
               << pEndInfo->get_tie_number()
               << " will be removed." << endl;
}

void TiesBuilder::error_no_start_tie(ImoTieDto* pEndInfo)
{
    m_reporter << "Line " << pEndInfo->get_line_number()
               << ". No 'start' element for tie number "
               << pEndInfo->get_tie_number()
               << ". Tie removed." << endl;
}

void TiesBuilder::error_no_end_tie(ImoTieDto* pStartInfo)
{
    m_reporter << "Line " << pStartInfo->get_line_number()
               << ". No 'stop' element for tie number "
               << pStartInfo->get_tie_number()
               << ". Tie removed." << endl;
}

void TiesBuilder::error_invalid_tie_old_syntax(int line)
{
    m_reporter << "Line " << line
               << ". No note found to match old syntax tie. Tie removed." << endl;
}

void TiesBuilder::remove_from_pending(ImoTieDto* pTieInfo)
{
    m_pendingTies.remove(pTieInfo);
}

void TiesBuilder::clear_pending_ties()
{
    std::list<ImoTieDto*>::iterator it;
    for (it = m_pendingTies.begin(); it != m_pendingTies.end(); ++it)
    {
        error_no_end_tie(*it);
        m_pAnalyser->remove_tie_element(*it);
    }
    m_pendingTies.clear();
}

void TiesBuilder::start_old_tie(ImoNote* pNote, LdpElement* pOldTie)
{
    if (m_pStartNoteTieOld)
        create_tie_if_old_syntax_tie_pending(pNote);

    m_pStartNoteTieOld = pNote;
    m_pOldTieParam = pOldTie;
}

void TiesBuilder::create_tie_if_old_syntax_tie_pending(ImoNote* pEndNote)
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
        m_pAnalyser->remove_old_tie_element(m_pOldTieParam);
        m_pStartNoteTieOld = NULL;
    }
}

void TiesBuilder::tie_notes(ImoNote* pStartNote, ImoNote* pEndNote)
{
    ImoTie* pTie = new ImoTie();
    pTie->set_tie_number(0);
    pTie->set_start_note( pStartNote );
    pTie->set_end_note( pEndNote );
    pTie->set_start_bezier(NULL);
    pTie->set_stop_bezier(NULL);

    pStartNote->set_tie_next(pTie);
    pEndNote->set_tie_prev(pTie);

    m_pBasicModel->add_tie(pTie);
}



//-------------------------------------------------------------------------------
// BeamsBuilder implementation
//-------------------------------------------------------------------------------

BeamsBuilder::BeamsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pBasicModel(pBasicModel)
{
}

BeamsBuilder::~BeamsBuilder()
{
    clear_pending_beams();
}

void BeamsBuilder::add_beam_info(ImoBeamInfo* pNewInfo)
{
    if (pNewInfo->is_end_of_beam())
        create_beam(pNewInfo);
    else
        save_beam_info(pNewInfo);
}

void BeamsBuilder::save_beam_info(ImoBeamInfo* pNewInfo)
{
    m_pendingBeams.push_back(pNewInfo);
}

void BeamsBuilder::create_beam(ImoBeamInfo* pEndInfo)
{
    int beamNum = pEndInfo->get_beam_number();
    if ( find_matching_info_items(beamNum) )
    {
        do_beam_notes_rests(pEndInfo);
        delete_consumed_info_items(pEndInfo);
    }
    else
    {
        error_no_matching_items(pEndInfo);
        delete_beam_element(pEndInfo);
    }
}

bool BeamsBuilder::find_matching_info_items(int beamNum)
{
    m_matches.clear();
    std::list<ImoBeamInfo*>::iterator it;
    for(it=m_pendingBeams.begin(); it != m_pendingBeams.end(); ++it)
    {
        if ((*it)->get_beam_number() == beamNum)
            m_matches.push_back(*it);
    }
    return !m_matches.empty();
}

void BeamsBuilder::do_beam_notes_rests(ImoBeamInfo* pEndInfo)
{
    m_matches.push_back(pEndInfo);

    ImoBeam* pBeam = new ImoBeam();
    std::list<ImoBeamInfo*>::iterator it;
    for (it = m_matches.begin(); it != m_matches.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        for (int level=0; level < 6; level++)
        {
            pNR->set_beam(pBeam);
            pNR->set_beam_type(level, (*it)->get_beam_type(level) );
        }
        pBeam->push_back(pNR);
    }

    m_pBasicModel->add_beam(pBeam);
}

void BeamsBuilder::delete_consumed_info_items(ImoBeamInfo* pEndInfo)
{
    m_matches.clear();
    int beamNum = pEndInfo->get_beam_number();
    std::list<ImoBeamInfo*>::iterator it;
    for(it=m_pendingBeams.begin(); it != m_pendingBeams.end(); )
    {
        if ((*it)->get_beam_number() == beamNum)
        {
            delete *it;
            it = m_pendingBeams.erase(it);
        }
        else
            ++it;
    }
    delete pEndInfo;
}

void BeamsBuilder::delete_beam_element(ImoBeamInfo* pInfo)
{
    LdpElement* pBeam = pInfo->get_beam_element();
    m_pAnalyser->erase_node(pBeam);
    //pInfo is deleted automatically when erasing node
}

void BeamsBuilder::clear_pending_beams()
{
    std::list<ImoBeamInfo*>::iterator it;
    for (it = m_pendingBeams.begin(); it != m_pendingBeams.end(); ++it)
    {
        error_no_end_beam(*it);
        m_pAnalyser->erase_node( (*it)->get_beam_element() );
    }
    m_pendingBeams.clear();
}

void BeamsBuilder::error_no_matching_items(ImoBeamInfo* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'begin/continue' elements for beam number "
               << pInfo->get_beam_number()
               << ". Beam removed." << endl;
}

void BeamsBuilder::error_no_end_beam(ImoBeamInfo* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'end' element for beam number "
               << pInfo->get_beam_number()
               << ". Beam removed." << endl;
}



//-------------------------------------------------------------------------------
// OldBeamsBuilder implementation
//-------------------------------------------------------------------------------

OldBeamsBuilder::OldBeamsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pBasicModel(pBasicModel)
{
}

OldBeamsBuilder::~OldBeamsBuilder()
{
    clear_pending_old_beams();
}

void OldBeamsBuilder::add_old_beam(ImoBeamInfo* pInfo)
{
    m_pendingOldBeams.push_back(pInfo);
}

void OldBeamsBuilder::clear_pending_old_beams()
{
    std::list<ImoBeamInfo*>::iterator it;
    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
    {
        error_no_end_old_beam(*it);
        //m_pAnalyser->erase_node( (*it)->get_beam_element() );
        delete *it;
    }
    m_pendingOldBeams.clear();
}

bool OldBeamsBuilder::is_old_beam_open()
{
    return m_pendingOldBeams.size() > 0;
}

void OldBeamsBuilder::error_no_end_old_beam(ImoBeamInfo* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No matching 'g-' element for 'g+'. Beam removed." << endl;
}

void OldBeamsBuilder::close_old_beam(ImoBeamInfo* pInfo)
{
    add_old_beam(pInfo);
    do_create_old_beam();
}

void OldBeamsBuilder::do_create_old_beam()
{
    ImoBeam* pBeam = new ImoBeam();
    std::list<ImoBeamInfo*>::iterator it;
    for (it = m_pendingOldBeams.begin(); it != m_pendingOldBeams.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        for (int level=0; level < 6; level++)
        {
            pNR->set_beam(pBeam);
            pNR->set_beam_type(level, (*it)->get_beam_type(level) );
        }
        pBeam->push_back(pNR);
        delete *it;
    }
    m_pendingOldBeams.clear();

    AutoBeamer autobeamer(pBeam);
    autobeamer.do_autobeam();

    m_pBasicModel->add_beam(pBeam);
}



//-------------------------------------------------------------------------------
// TupletsBuilder implementation
//-------------------------------------------------------------------------------

TupletsBuilder::TupletsBuilder(ostream& reporter, BasicModel* pBasicModel, Analyser* pAnalyser)
    : m_reporter(reporter)
    , m_pAnalyser(pAnalyser)
    , m_pBasicModel(pBasicModel)
{
}

TupletsBuilder::~TupletsBuilder()
{
    clear_pending_tuplets();
}

void TupletsBuilder::add_tuplet_info(ImoTupletDto* pNewInfo)
{
    if (pNewInfo->is_end_of_tuplet())
        create_tuplet(pNewInfo);
    else
        save_tuplet_info(pNewInfo);
}

void TupletsBuilder::save_tuplet_info(ImoTupletDto* pNewInfo)
{
    m_pendingTuplets.push_back(pNewInfo);
}

void TupletsBuilder::create_tuplet(ImoTupletDto* pEndInfo)
{
    save_tuplet_info(pEndInfo);

    ImoTuplet* pTuplet = new ImoTuplet();
    std::list<ImoTupletDto*>::iterator it;
    for (it = m_pendingTuplets.begin(); it != m_pendingTuplets.end(); ++it)
    {
        ImoNoteRest* pNR = (*it)->get_note_rest();
        pNR->set_tuplet(pTuplet);
        pTuplet->push_back(pNR);
        delete *it;
    }
    m_pendingTuplets.clear();

    m_pBasicModel->add_tuplet(pTuplet);
}

void TupletsBuilder::clear_pending_tuplets()
{
    std::list<ImoTupletDto*>::iterator it;
    for (it = m_pendingTuplets.begin(); it != m_pendingTuplets.end(); ++it)
    {
        error_no_end_tuplet(*it);
        LdpElement* pElm = (*it)->get_tuplet_element();
        if (!pElm || pElm->get_imo() == NULL)
            delete *it;
        m_pAnalyser->erase_node( pElm );
    }
    m_pendingTuplets.clear();
}

void TupletsBuilder::error_no_end_tuplet(ImoTupletDto* pInfo)
{
    m_reporter << "Line " << pInfo->get_line_number()
               << ". No 'end' element for tuplet. Tuplet removed." << endl;
}


}   //namespace lomse
