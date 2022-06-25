//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_mnx_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_mxl_exporter.h"
#include "lomse_logger.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"

#include <cmath>        //round
#include <stack>
#include <list>
#include <algorithm>
using namespace std;

namespace lomse
{

//=======================================================================================
// Helper struct to save MusicXML data for a barline
//=======================================================================================
struct BarlineData
{
    int times = 0;
    int winged = k_wings_none;
    bool fForward = true;
    string style;
    string location;
};


//=======================================================================================
// MxlGenerator
//=======================================================================================
class MxlGenerator
{
protected:
    MxlExporter* m_pExporter;
    stringstream m_source;

public:
    MxlGenerator(MxlExporter* pExporter);
    virtual ~MxlGenerator() {}

    virtual string generate_source() = 0;

protected:
    void start_element(const string& name, ImoObj* pImo=nullptr);
    void start_element_no_attribs(const string& name, ImoObj* pImo=nullptr);
    void empty_element(const string& name, ImoObj* pImo=nullptr);
    void create_element(const string& name, const string& content);
    void create_element(const string& name, int content);
    void close_start_tag();
    void start_attrib(const string& name);
    void end_attrib();
    void add_attribute(const string& name, const string& value);
    void add_attribute(const string& name, int value);
    void end_element(bool fInNewLine = true, bool fNewElement=true);
    void add_separator_line();
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void new_line();
    void start_element_if_not_started(const std::string& tag);
    void end_element_if_started(const std::string& tag);
    void add_source_for(ImoObj* pImo);
    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void source_for_inline_level_object(ImoInlineLevelObj* pImo,
                                        bool fFirstItem, ImoStyle* pParentStyle);
    void increment_indent();
    void decrement_indent();

    void add_optional_style(ImoContentObj* pObj);

    void add_staff(ImoStaffObj* pSO);

};

const bool k_in_same_line = false;
//const bool k_in_new_line = true;
const int k_indent_step = 3;

//=======================================================================================
// generators for specific elements
//=======================================================================================

//---------------------------------------------------------------------------------------
class xxxxxxMxlGenerator : public MxlGenerator
{
protected:
    //ImoXXXXX* m_pObj;

public:
    xxxxxxMxlGenerator(ImoObj* UNUSED(pImo), MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        //m_pObj = static_cast<ImoXXXXX*>(pImo);
    }

    string generate_source() override
    {
        //start_element("xxxxx", m_pObj);
        close_start_tag();
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class MetronomeMxlGenerator : public MxlGenerator
{
protected:
    ImoMetronomeMark* m_pObj;

public:
    MetronomeMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMetronomeMark*>(pImo);
    }

    string generate_source() override
    {
        return m_source.str();
    }

protected:

};


//---------------------------------------------------------------------------------------
class BarlineMxlGenerator : public MxlGenerator
{
protected:
    ImoBarline*  m_pObj;
    BarlineData  m_right;       //data for right barline
    BarlineData  m_left;        //data for left barline in next measure

public:
    BarlineMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    string generate_source() override
    {
        m_right.style = determine_right_barline_style(m_pObj->get_type());
        m_right.times = m_pObj->get_num_repeats();
        m_right.fForward = false;
        m_right.winged = m_pObj->get_winged();
        m_right.location = m_pObj->is_middle() ? "middle" : "right";

        generate_source_for_barline(m_right);
        prepare_left_barline();

        return m_source.str();
    }

    string generate_left_barline()
    {
        BarlineData data = m_pExporter->get_data_for_left_barline();
        m_pExporter->clear_data_for_left_barline();
        return generate_source_for_barline(data);
    }


protected:

    //-----------------------------------------------------------------------------------
    string generate_source_for_barline(const BarlineData& data)
    {
        //@ <!ELEMENT barline (bar-style?, %editorial;, wavy-line?,
        //@     segno?, coda?, (fermata, fermata?)?, ending?, repeat?)>

        if (data.style.empty())
            return "";

        start_element("barline", m_pObj);
        add_attributes(data);
        close_start_tag();

        //bar-style?
        create_element("bar-style", data.style);

        if (data.style != "none")
        {
            //TODO
            //%editorial;
            //wavy-line?,
            //segno?
            //coda?
            //(fermata, fermata?)?
            //ending?

            //repeat?
            add_repeat(data.fForward, data.winged, data.times);

            //source_for_base_staffobj(m_pObj);
        }

        end_element();  //barline

        return m_source.str();
    }

    //-----------------------------------------------------------------------------------
    void add_attributes(const BarlineData& data)
    {
        //location (right | left | middle) "right"
        if (data.location != "right")
            add_attribute("location", data.location);

        //TODO
        //segno CDATA #IMPLIED
        //coda CDATA #IMPLIED
        //divisions CDATA #IMPLIED
    }

    //-----------------------------------------------------------------------------------
    string determine_right_barline_style(int type)
    {
        if (!m_pObj->is_visible())
                return "none";

        switch (type)
        {
            case k_barline_simple:
                return "";
            case k_barline_none:
                return "none";
            case k_barline_double:
                return "light-light";
            case k_barline_end:
                return "light-heavy";
//            case k_barline_start:
            case k_barline_start_repetition:
                m_left.style = "heavy-light";
                m_left.fForward = true;
                m_left.location = "left";
                //TODO: in current IM times is only on right barline, end_repetion / double_repetition
                m_left.times = 1;
                m_left.winged = m_right.winged;
                return "";
            case k_barline_end_repetition:
                return "light-heavy";
            case k_barline_double_repetition:
            case k_barline_double_repetition_alt:
                m_left.style = "heavy-light";
                m_left.fForward = true;
                m_left.location = "left";
                //TODO: in current IM times is only on right barline, end_repetion / double_repetition
                m_left.times = 1;
                m_left.winged = m_right.winged;
                return "light-heavy";
            default:
                return "";
        }
//                return "dashed";
//                return "dotted";
//                return "heavy";
    }

    //-----------------------------------------------------------------------------------
    void prepare_left_barline()
    {
        if (m_left.style.empty())
            return;

        m_pExporter->save_data_for_left_barline(m_left);
    }

    //-----------------------------------------------------------------------------------
    void add_repeat(bool fForward, int winged, int times)
    {
        //@ <repeat>
        //@ <!ELEMENT repeat EMPTY>
        //@ <!ATTLIST repeat
        //@     direction (backward | forward) #REQUIRED
        //@     times CDATA #IMPLIED
        //@     winged (none | straight | curved |
        //@         double-straight | double-curved) #IMPLIED
        //@ >

        if (times == 0)
            return;

        start_element("repeat");

        add_attribute("direction", fForward ? "forward" : "backward");

        if (times > 1)
            add_attribute("times", times);

        string w;
        switch(winged)
        {
            case k_wings_none:              w="none";               break;
            case k_wings_straight:          w="straight";           break;
            case k_wings_curved:            w="curved";             break;
            case k_wings_double_straight:   w="double-straight";    break;
            case k_wings_double_curved:     w="double-curved";      break;
        }
        if (winged != k_wings_none)
            add_attribute("winged", w);

        end_element(false, false);
    }

};


//---------------------------------------------------------------------------------------
class BeamMxlGenerator : public MxlGenerator
{
protected:
    ImoNoteRest* m_pNR;
    ImoBeam* m_pBeam;

public:
    BeamMxlGenerator(ImoObj* pImo, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pNR = static_cast<ImoNoteRest*>(pImo);
        m_pBeam = m_pNR->get_beam();
    }

    string generate_source() override
    {
        //skip if note in chord and not base of chord
        if (m_pNR->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(m_pNR);
            if (pNote->is_in_chord() && ! pNote->is_start_of_chord())
                return "";
        }

        add_source();
        return m_source.str();
    }

protected:

    void add_source()
    {
        for (int i=0; i < 6; ++i)
        {
            int type = m_pNR->get_beam_type(i);
            if (type == ImoBeam::k_none)
                break;
            else if (type == ImoBeam::k_begin)
                add_beam(i+1, "begin");
            else if (type == ImoBeam::k_continue)
                add_beam(i+1, "continue");
            else if (type == ImoBeam::k_end)
                add_beam(i+1, "end");
            else if (type == ImoBeam::k_forward)
                add_beam(i+1, "forward hook");
            else if (type == ImoBeam::k_backward)
                add_beam(i+1, "backward hook");
        }
    }

    void add_beam(int number, const string& value)
    {
        start_element("beam", m_pBeam);
        add_attribute("number", number);
        close_start_tag();
        m_source << value;
        end_element(k_in_same_line);
    }

};


//---------------------------------------------------------------------------------------
class ClefMxlGenerator : public MxlGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    string generate_source() override
    {
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
        start_element("clef", m_pObj);
        add_staff_number();
        //add_attributes();
        close_start_tag();

        add_sign_and_line();
        //source_for_base_staffobj(m_pObj);

        end_element();  //clef
        return m_source.str();
    }

protected:

    void add_staff_number()
    {
        ImoInstrument* pInsr = m_pExporter->get_current_instrument();
        if (pInsr && pInsr->get_num_staves() > 1)
        {
            add_attribute("number", m_pObj->get_staff() + 1);
        }
    }

    void add_sign_and_line()
    {
        start_element_no_attribs("sign");
        int sign = m_pObj->get_sign();
        switch(sign)
        {
            case k_clef_sign_G:     m_source << "G";    break;
            case k_clef_sign_F:     m_source << "F";    break;
            case k_clef_sign_C:     m_source << "C";    break;
            case k_clef_sign_percussion:     m_source << "percussion";    break;
            case k_clef_sign_TAB:   m_source << "TAB";    break;
            default:
                m_source << "none";
        }
        end_element(k_in_same_line);  //sign

        if (sign == k_clef_sign_G || sign == k_clef_sign_F || sign == k_clef_sign_C)
        {
            create_element("line", m_pObj->get_line());

            int octaveChange = m_pObj->get_octave_change();
            if (octaveChange != 0)
                create_element("clef-octave-change", octaveChange);
        }
    }

};


//---------------------------------------------------------------------------------------
class ContentObjMxlGenerator : public MxlGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoContentObj*>(pImo);
    }

    string generate_source() override
    {
//        add_user_location();
//        add_attachments();
//        source_for_base_imobj(m_pObj);
        return m_source.str();
    }

protected:

//    void add_user_location()
//    {
//        Tenths ux = m_pObj->get_user_location_x();
//        if (ux != 0.0f)
//            m_source << " (dx " << MxlExporter::float_to_string(ux) << ")";
//
//        Tenths uy = m_pObj->get_user_location_y();
//        if (uy != 0.0f)
//            m_source << " (dy " << MxlExporter::float_to_string(uy) << ")";
//    }
//
//    void add_attachments()
//    {
//        if (m_pObj->get_num_attachments() > 0)
//        {
//            increment_indent();
////            std::list<ImoAuxObj*>& attachments = m_pObj->get_attachments();
////            std::list<ImoAuxObj*>::iterator it;
////            for (it = attachments.begin(); it != attachments.end(); ++it)
////            {
////                ImoAuxObj* pAuxObj = *it;
////                if ( pAuxObj->is_relobj() )
////                {
////                    ImRelObj* pRO = dynamic_cast<ImRelObj*)>(pAuxObj);
////
////                    //exclude beams, as source code for them is generted in ImoNote.
////                    //AWARE. This is necessary because MNX parser needs to have beam
////                    //info to crete the note, before it can process any other attachment.
////                    //Therefore, it was decided to generate beam tag before generating
////                    //attachment tags.
////                    if (!pRO->IsBeam())
////                    {
////                        if ( pRO->GetStartNoteRest() == (lmNoteRest*)this )
////                            m_source += pRO->SourceMNX_First(nIndent, fUndoData, (lmNoteRest*)this);
////                        else if ( pRO->GetEndNoteRest() == (lmNoteRest*)this )
////                            m_source += pRO->SourceMNX_Last(nIndent, fUndoData, (lmNoteRest*)this);
////                        else
////                            m_source += pRO->SourceMNX_Middle(nIndent, fUndoData, (lmNoteRest*)this);
////                    }
////                }
////                else if ( pAuxObj->IsRelObX() )
////                {
////                    lmRelObX* pRO = (lmRelObX*)pAuxObj;
////
////                    //exclude beams, as source code for them is generted in ImoNote.
////                    //AWARE. This is necessary because MNX parser needs to have beam
////                    //info to crete the note, before it can process any other attachment.
////                    //Therefore, it was decided to generate beam tag before generating
////                    //attachment tags.
////                    if (!pRO->IsBeam())
////                    {
////                        if (pRO->GetStartSO() == this)
////                            m_source += pRO->SourceMNX_First(nIndent, fUndoData, this);
////                        else if (pRO->GetEndSO() == this)
////                            m_source += pRO->SourceMNX_Last(nIndent, fUndoData, this);
////                        else
////                            m_source += pRO->SourceMNX_Middle(nIndent, fUndoData, this);
////                    }
////                }
////                else
////                    source_for_auxobj(pAuxObj);
////            }
//            decrement_indent();
//        }
//    }
};


//---------------------------------------------------------------------------------------
class DefineStyleMxlGenerator : public MxlGenerator
{
protected:
    ImoStyle* m_pObj;

public:
    DefineStyleMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStyle*>(pImo);
    }

    string generate_source() override
    {
//        start_element("defineStyle", m_pObj);
//        close_start_tag();
//        add_name();
//        add_properties();
//        end_element();
        return m_source.str();
    }

protected:

//    void add_name()
//    {
//        create_element("name", m_pObj->get_name());
//    }
//
//    void add_properties()
//    {
//        string sValue;
//        LUnits uValue;
//        int iValue;
//        float rValue;
//        Color cValue;
//
//        //font properties
//        if (m_pObj->get_string_property(ImoStyle::k_font_file, &sValue))
//        {
//            if (!sValue.empty())     //font-file can be empty when font-name is set
//                create_string_element("font-file", sValue);
//        }
//
//        if (m_pObj->get_string_property(ImoStyle::k_font_name, &sValue))
//            create_string_element("font-name", sValue);
//
//        if (m_pObj->get_float_property(ImoStyle::k_font_size, &rValue))
//        {
//            start_element("font-size", nullptr);
//            close_start_tag();
//            m_source << rValue << "pt";
//            end_element(k_in_same_line);
//        }
//
//        if (m_pObj->get_int_property(ImoStyle::k_font_style, &iValue))
//            create_font_style(iValue);
//
//        if (m_pObj->get_int_property(ImoStyle::k_font_weight, &iValue))
//            create_font_weight(iValue);
//
//        //text properties
//        if (m_pObj->get_int_property(ImoStyle::k_word_spacing, &iValue))
//            create_int_element("word-spacing", iValue);
//
//        if (m_pObj->get_int_property(ImoStyle::k_text_decoration, &iValue))
//            create_text_decoration(iValue);
//
//        if (m_pObj->get_int_property(ImoStyle::k_vertical_align, &iValue))
//            create_vertical_align(iValue);
//
//        if (m_pObj->get_int_property(ImoStyle::k_text_align, &iValue))
//            create_text_align(iValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_text_indent_length, &uValue))
//            create_lunits_element("", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_word_spacing_length, &uValue))
//            create_lunits_element("", uValue);
//
//        if (m_pObj->get_float_property(ImoStyle::k_line_height, &rValue))
//            create_float_element("line-height", rValue);
//
//        //color and background properties
//        if (m_pObj->get_color_property(ImoStyle::k_color, &cValue))
//            create_color_element("color", cValue);
//
//        if (m_pObj->get_color_property(ImoStyle::k_background_color, &cValue))
//            create_color_element("background-color", cValue);
//
//        //border-width  properties
//        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_top, &uValue))
//            create_lunits_element("border-width-top", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_bottom, &uValue))
//            create_lunits_element("border-width-bottom", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_left, &uValue))
//            create_lunits_element("border-width-left", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_right, &uValue))
//            create_lunits_element("border-width-right", uValue);
//
//        //padding properties
//        if (m_pObj->get_lunits_property(ImoStyle::k_padding_top, &uValue))
//            create_lunits_element("padding-top", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_padding_bottom, &uValue))
//            create_lunits_element("padding-bottom", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_padding_left, &uValue))
//            create_lunits_element("padding-left", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_padding_right, &uValue))
//            create_lunits_element("padding-right", uValue);
//
//        //margin properties
//        if (m_pObj->get_lunits_property(ImoStyle::k_margin_top, &uValue))
//            create_lunits_element("margin-top", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_margin_bottom, &uValue))
//            create_lunits_element("margin-bottom", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_margin_left, &uValue))
//            create_lunits_element("margin-left", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_margin_right, &uValue))
//            create_lunits_element("margin-right", uValue);
//
//        //size properties
//        if (m_pObj->get_lunits_property(ImoStyle::k_width, &uValue))
//            create_lunits_element("width", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_height, &uValue))
//            create_lunits_element("height", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_min_width, &uValue))
//            create_lunits_element("min-width", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_min_height, &uValue))
//            create_lunits_element("min-height", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_max_width, &uValue))
//            create_lunits_element("max-width", uValue);
//
//        if (m_pObj->get_lunits_property(ImoStyle::k_max_height, &uValue))
//            create_lunits_element("max-height", uValue);
//
//        //table properties
//        if (m_pObj->get_lunits_property(ImoStyle::k_table_col_width, &uValue))
//            create_lunits_element("table-col-width", uValue);
//
//    }
//
//    void create_string_element(const string& tag, const string& value)
//    {
//        start_element(tag, nullptr);
//        close_start_tag();
//        m_source << value;
//        end_element(k_in_same_line);
//    }
//
//    void create_float_element(const string& tag, float value)
//    {
//        start_element(tag, nullptr);
//        close_start_tag();
//        m_source << value;
//        end_element(k_in_same_line);
//    }
//
//    void create_lunits_element(const string& tag, LUnits value)
//    {
//        start_element(tag, nullptr);
//        close_start_tag();
//        m_source << value;
//        end_element(k_in_same_line);
//    }
//
//    void create_int_element(const string& tag, int value)
//    {
//        start_element(tag, nullptr);
//        close_start_tag();
//        m_source << value;
//        end_element(k_in_same_line);
//    }
//
//    void create_color_element(const string& tag, Color color)
//    {
//        start_element(tag, nullptr);
//        close_start_tag();
//
//        m_source << MxlExporter::color_to_mnx(color);
//
//        end_element(k_in_same_line);
//    }
//
//    void create_font_style(int value)
//    {
//        start_element("font-style", nullptr);
//        close_start_tag();
//
//        if (value == ImoStyle::k_font_style_normal)
//            m_source << "normal";
//        else if (value == ImoStyle::k_font_style_italic)
//            m_source << "italic";
//        else
//            m_source << "invalid value " << value;
//
//        end_element(k_in_same_line);
//    }
//
//    void create_font_weight(int value)
//    {
//        start_element("font-weight", nullptr);
//        close_start_tag();
//
//        if (value == ImoStyle::k_font_weight_normal)
//            m_source << "normal";
//        else if (value == ImoStyle::k_font_weight_bold)
//            m_source << "bold";
//        else
//            m_source << "invalid value " << value;
//
//        end_element(k_in_same_line);
//    }
//
//    void create_text_decoration(int value)
//    {
//        start_element("text-decoration", nullptr);
//        close_start_tag();
//
//        if (value == ImoStyle::k_decoration_none)
//            m_source << "none";
//        else if (value == ImoStyle::k_decoration_underline)
//            m_source << "underline";
//        else if (value == ImoStyle::k_decoration_overline)
//            m_source << "overline";
//        else if (value == ImoStyle::k_decoration_line_through)
//            m_source << "line-through";
//        else
//            m_source << "invalid value " << value;
//
//        end_element(k_in_same_line);
//    }
//
//    void create_vertical_align(int value)
//    {
//        start_element("vertical-align", nullptr);
//        close_start_tag();
//
//        if (value == ImoStyle::k_valign_baseline)
//            m_source << "baseline";
//        else if (value == ImoStyle::k_valign_sub)
//            m_source << "sub";
//        else if (value == ImoStyle::k_valign_super)
//            m_source << "super";
//        else if (value == ImoStyle::k_valign_top)
//            m_source << "top";
//        else if (value == ImoStyle::k_valign_text_top)
//            m_source << "text-top";
//        else if (value == ImoStyle::k_valign_middle)
//            m_source << "middle";
//        else if (value == ImoStyle::k_valign_bottom)
//            m_source << "bottom";
//        else if (value == ImoStyle::k_valign_text_bottom)
//            m_source << "text-bottom";
//        else
//            m_source << "invalid value " << value;
//
//        end_element(k_in_same_line);
//    }
//
//    void create_text_align(int value)
//    {
//        start_element_no_attribs("text-align");
//
//        if (value == ImoStyle::k_align_left)
//            m_source << "left";
//        else if (value == ImoStyle::k_align_right)
//            m_source << "right";
//        else if (value == ImoStyle::k_align_center)
//            m_source << "center";
//        else if (value == ImoStyle::k_align_justify)
//            m_source << "justify";
//        else
//            m_source << "invalid value " << value;
//
//        end_element(k_in_same_line);
//    }

};


//---------------------------------------------------------------------------------------
class ErrorMxlGenerator : public MxlGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorMxlGenerator(ImoObj* pImo, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source() override
    {
        start_element_no_attribs("TODO", m_pImo);
        m_source << "Error: no MxlExporter for Imo=" << m_pImo->get_name()
                 << ", Imo type=" << m_pImo->get_obj_type()
                 << ", id=" << m_pImo->get_id();
        end_element(k_in_same_line);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class FermataMxlGenerator : public MxlGenerator
{
protected:
    ImoFermata* m_pImo;

public:
    FermataMxlGenerator(ImoObj* pImo, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo(static_cast<ImoFermata*>(pImo))
    {
    }

    string generate_source() override
    {
        start_element("fermata");
        if (m_pImo->get_placement() == k_placement_below)
            add_attribute("type", "inverted");
        end_element(false, false);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ImObjMxlGenerator : public MxlGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = pImo;
    }

    string generate_source() override
    {
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class InstrumentMxlGenerator : public MxlGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoInstrument*>(pImo);
        m_pExporter->set_current_instrument(m_pObj);
    }

    string generate_source() override
    {
        //<part> = <measure>*
        start_element("part", m_pObj);
        add_attribute("id", m_pObj->get_instr_id());
        close_start_tag();
        add_music_data();
        end_element();  //part
        return m_source.str();
    }

protected:

    void add_music_data()
    {
        add_source_for( m_pObj->get_musicdata() );
    }
};


//---------------------------------------------------------------------------------------
class KeySignatureMxlGenerator : public MxlGenerator
{
protected:
    ImoKeySignature* m_pObj;

public:
    KeySignatureMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoKeySignature*>(pImo);
    }

    string generate_source() override
    {
        //@ <!ELEMENT key (((cancel?, fifths, mode?) |
        //@ 	((key-step, key-alter, key-accidental?)*)), key-octave*)>
        //@ <!ATTLIST key
        //@     number CDATA #IMPLIED
        //@     %print-style;
        //@     %print-object;
        //@     %optional-unique-id;
        //@ >

        start_element_no_attribs("key", m_pObj);
        add_fifths_mode();

        end_element();
        return m_source.str();
    }

protected:

    void add_fifths_mode()
    {
        if (m_pObj->is_standard())
        {
            create_element("fifths", m_pObj->get_fifths());

            int mode = m_pObj->get_mode();
            if (mode != k_key_mode_none)
            {
                start_element_no_attribs("mode");
                switch(mode)
                {
                    case k_key_mode_major:      m_source << "major"; break;
                    case k_key_mode_minor:      m_source << "minor"; break;
                    case k_key_mode_dorian:     m_source << "dorian"; break;
                    case k_key_mode_phrygian:   m_source << "phrygian"; break;
                    case k_key_mode_lydian:     m_source << "lydian"; break;
                    case k_key_mode_mixolydian: m_source << "mixolydian"; break;
                    case k_key_mode_aeolian:    m_source << "aeolian"; break;
                    case k_key_mode_ionian:     m_source << "ionian"; break;
                    case k_key_mode_locrian:    m_source << "locrian"; break;
                    default:
                        break;
                }
                end_element(k_in_same_line);
            }
        }
        else
        {
        }
    }

};


//---------------------------------------------------------------------------------------
class LenmusdocMxlGenerator : public MxlGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source() override
    {
        m_source << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        m_source << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\" "
            << "\"http://www.musicxml.org/dtds/partwise.dtd\">";

        add_comment();
        add_content();
        return m_source.str();
    }

protected:

    void add_comment()
    {
        if (!m_pExporter->get_remove_newlines())
        {
            start_comment();
            m_source << "MusicXML file generated by Lomse, version "
                     << m_pExporter->get_version_and_time_string();
            end_comment();
        }
    }

    void add_content()
    {
        ImoContent* pContent = m_pObj->get_content();
        ImoObj::children_iterator it;
        for (it= pContent->begin(); it != pContent->end(); ++it)
        {
            if ((*it)->is_score())
            {
                ImoScore* pScore = static_cast<ImoScore*>(*it);
                add_source_for( pScore );
                return;
            }
        }
    }

};


//---------------------------------------------------------------------------------------
class MusicDataMxlGenerator : public MxlGenerator
{
protected:
    ImoMusicData* m_pObj;
    ImoScore* m_pScore;

public:
    MusicDataMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
        m_pScore = m_pExporter->get_current_score();
    }

    string generate_source() override
    {
        add_staffobjs();
        return m_source.str();
    }

protected:

    int m_curMeasure;
    int m_iInstr;
    vector<int> m_lines;
    ColStaffObjs* m_pColStaffObjs;
    ColStaffObjsIterator m_itStartOfMeasure;
    ColStaffObjsIterator m_itEndOfMeasure;
    bool m_fDivisionsAdded = false;

    const int k_max_num_lines = 16;

    //-----------------------------------------------------------------------------------
    void add_staffobjs()
    {
        ImoInstrument* pInstr = m_pExporter->get_current_instrument();
        m_iInstr = m_pScore->get_instr_number_for(pInstr);
        m_pColStaffObjs = m_pScore->get_staffobjs_table();

        if (m_pColStaffObjs->num_entries() < 1)
            return;

        m_lines.reserve(k_max_num_lines);
        ColStaffObjsIterator it = m_pColStaffObjs->begin();
        m_curMeasure = (*it)->measure();
        while (it != m_pColStaffObjs->end())
        {
            m_itStartOfMeasure = it;
            determine_how_many_lines_in_current_measure();
            add_measure_element(m_curMeasure+1);
            add_attributes_at_start_of_measure();
            add_source_for_this_measure();
            add_source_for_barline();
            it = m_itEndOfMeasure;
            end_element();  //measure
        }
    }

    //-----------------------------------------------------------------------------------
    void add_attributes_at_start_of_measure()
    {
        ColStaffObjsIterator it;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            if ((*it)->num_instrument() == m_iInstr && (*it)->measure() == m_curMeasure)
            {
                if (!m_fDivisionsAdded)
                    add_divisions();

                ImoStaffObj* pImo = (*it)->imo_object();
                if (is_greater_time(pImo->get_duration(), 0.0))
                {
                    m_itStartOfMeasure = it;
                    break;
                }

                add_attributes(it);
                break;
            }
        }

        end_element_if_started("attributes");
    }

    //-----------------------------------------------------------------------------------
    void add_measure_element(int nMeasure)
    {
        add_separator_line();
        start_element("measure");
        start_attrib("number");
        m_source << nMeasure;
        end_attrib();
        close_start_tag();
        m_pExporter->set_current_timepos(0);

        if (m_pExporter->left_barline_required())
            add_left_barline();
    }

    //-----------------------------------------------------------------------------------
    void add_left_barline()
    {
        BarlineMxlGenerator exporter(nullptr, m_pExporter);
        m_source << exporter.generate_left_barline();
    }

    //-----------------------------------------------------------------------------------
    void add_divisions()
    {
        start_element_no_attribs("attributes");
        create_element("divisions", m_pExporter->get_divisions());
        m_fDivisionsAdded = true;
    }

    //-----------------------------------------------------------------------------------
    void add_attributes(ColStaffObjsIterator& it)
    {
        //@ <!ELEMENT attributes (%editorial;, divisions?, key*, time*,
        //@     staves?, part-symbol?, instruments?, clef*, staff-details*,
        //@     transpose*, directive*, measure-style*)>

        list<ImoObj*> clefs;
        list<ImoObj*> keys;
        list<ImoObj*> times;

        //collect clefs, keys and time signatures
        while (it != m_itEndOfMeasure)
        {
            if ((*it)->num_instrument() == m_iInstr)
            {
                ImoStaffObj* pImo = (*it)->imo_object();
                if (pImo->is_clef())
                    clefs.push_back(pImo);
                else if (pImo->is_key_signature() || pImo->is_time_signature())
                {
                    if ((*it)->staff() == 0)    //ignore 'ghost' key/time signatures
                    {
                        if (pImo->is_key_signature())
                            keys.push_back(pImo);
                        else if (pImo->is_time_signature())
                            times.push_back(pImo);
                    }
                }
                else
                    break;
            }

            ++it;
        }
        m_itStartOfMeasure = it;        //advance start: to first non clef key, time

        //add to attributes
        if (clefs.size() > 0 || keys.size() > 0 || times.size() > 0)
        {
            start_element_if_not_started("attributes");

            //key*
            for (auto obj : keys)
                add_source_for(obj);

            //time*
            for (auto obj : times)
                add_source_for(obj);

            //staves?
            ImoInstrument* pInsr = m_pExporter->get_current_instrument();
            if (pInsr && pInsr->get_num_staves() > 1)
            {
                create_element("staves", pInsr->get_num_staves());
            }

            //TODO: part-symbol?

            //TODO: instruments?

            //clef*
            for (auto obj : clefs)
                add_source_for(obj);
        }

        //TODO: add staff-details*, transpose*, directive*, measure-style*
    }

    //-----------------------------------------------------------------------------------
    void determine_how_many_lines_in_current_measure()
    {
        m_lines.assign(k_max_num_lines, 0);
        ColStaffObjsIterator it = m_itStartOfMeasure;
        if (it != m_pColStaffObjs->end())
        {
            while (it != m_pColStaffObjs->end() && (*it)->num_instrument() != m_iInstr)
                ++it;
            m_itStartOfMeasure = it;

            if (it != m_pColStaffObjs->end())
                m_curMeasure = (*it)->measure();

            while (it != m_pColStaffObjs->end()
                   && !(*it)->imo_object()->is_barline()
                   && (*it)->measure() == m_curMeasure)
            {
                if ((*it)->num_instrument() == m_iInstr)
                    m_lines[ (*it)->line() ] = 1;

                ++it;
            }
        }

        m_itEndOfMeasure = it;
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_this_measure()
    {
        bool fFirstLine = true;
        for (int i=0; i < k_max_num_lines; ++i)
        {
            if (m_lines[i] != 0)
            {
                if (!fFirstLine)
                    add_backup();
                add_source_for_line(i);
                fFirstLine = false;
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_barline()
    {
        //TODO
        ColStaffObjsIterator it = m_itEndOfMeasure;
        if (it != m_pColStaffObjs->end() && (*it)->imo_object()->is_barline()
            && (*it)->num_instrument() == m_iInstr)
        {
//            add_fwd_backup_if_needed( (*it)->time() );
            add_source_for((*it)->imo_object());
        }

        //advance to start of next measure
        while (it != m_pColStaffObjs->end() && (*it)->measure() == m_curMeasure)
        {
            ++it;
        }
        m_itEndOfMeasure = it;
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_line(int line)
    {
        ColStaffObjsIterator it;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            ColStaffObjsEntry* pEntry = *it;
            if (pEntry->num_instrument() == m_iInstr && pEntry->line() == line)
                add_source_for( pEntry->imo_object() );
        }
    }

    //-----------------------------------------------------------------------------------
    void add_backup()
    {
        start_element_no_attribs("backup");
        create_element("duration", m_pExporter->get_current_timepos());
        end_element();  //backup
    }

};


//---------------------------------------------------------------------------------------
class SlurMxlGenerator : public MxlGenerator
{
protected:
    ImoSlur* m_pSlur;
    ImoNote* m_pNote;

public:
    SlurMxlGenerator(ImoObj* pSlur, ImoNote* pNote, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pNote(pNote)
    {
        m_pSlur = static_cast<ImoSlur*>(pSlur);
    }

    string generate_source() override
    {
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
        if (m_pNote == m_pSlur->get_start_object())
        {
            start_element_if_not_started("notations");
            start_element("slur");
            add_attribute("number", m_pExporter->start_slur_and_get_number(m_pSlur->get_id()));
            add_attribute("type", "start");

            //TODO
            //%line-type;
            //%dashed-formatting;
            //%position;

            //%placement;
            //%orientation;
//            //TODO: placement is not correctly imported and interferes with orientation
//            if (pSlur->get_orientation() != k_orientation_default)
//            {
//                add_attribute("orientation",
//                    pSlur->get_orientation() == k_orientation_over ? "over" : "under");
//            }

            //TODO
            //%bezier;
            //%color;

            end_element(false, false);  //slur
        }
        else if (m_pNote == m_pSlur->get_end_object())
        {
            start_element_if_not_started("notations");
            start_element("slur");
            add_attribute("number", m_pExporter->close_slur_and_get_number(m_pSlur->get_id()));
            add_attribute("type", "stop");
            end_element(false, false);  //slur
        }
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class TupletMxlGenerator : public MxlGenerator
{
protected:
    ImoTuplet* m_pTuplet;
    ImoNoteRest* m_pNR;

public:
    TupletMxlGenerator(ImoObj* pTuplet, ImoNoteRest* pNR, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pNR(pNR)
    {
        m_pTuplet = static_cast<ImoTuplet*>(pTuplet);
    }

    string generate_source() override
    {
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

        if (m_pNR == m_pTuplet->get_start_object())
        {
            start_element_if_not_started("notations");
            start_element("tuplet");
            add_attribute("type", "start");
            add_attribute("number", m_pExporter->start_tuplet_and_get_number(m_pTuplet->get_id()));

            if (m_pTuplet->get_show_bracket() != k_yesno_default)
            {
                add_attribute("bracket",
                    m_pTuplet->get_show_bracket() == k_yesno_yes ? "yes" : "no");
            }

            if (m_pTuplet->get_placement() != k_placement_default)
            {
                add_attribute("placement",
                    m_pTuplet->get_placement() == k_placement_above ? "above" : "below");
            }

            if (m_pTuplet->get_show_number() == ImoTuplet::k_number_none)
            {
                add_attribute("show-number", "none");
                end_element(false, false);  //tuplet
            }
            else
            {
                if (m_pTuplet->get_show_number() == ImoTuplet::k_number_both)
                    add_attribute("show-number", "both");
                else
                    add_attribute("show-number", "actual");

                close_start_tag();
                start_element_no_attribs("tuplet-actual");
                create_element("tuplet-number", m_pTuplet->get_actual_number());
                end_element();    //tuplet-actual
                start_element_no_attribs("tuplet-normal");
                create_element("tuplet-number", m_pTuplet->get_normal_number());
                end_element();    //tuplet-normal
                end_element();    //tuplet
            }
        }
        else if (m_pNR == m_pTuplet->get_end_object())
        {
            start_element_if_not_started("notations");
            start_element("tuplet");
            add_attribute("type", "stop");
            add_attribute("number", m_pExporter->close_tuplet_and_get_number(m_pTuplet->get_id()));
            end_element(false, false);  //tuplet
        }
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class NoteRestMxlGenerator : public MxlGenerator
{
protected:
    ImoNoteRest* m_pNR;
    ImoNote* m_pNote = nullptr;
    ImoRest* m_pRest = nullptr;

public:
    NoteRestMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pNR = static_cast<ImoNoteRest*>(pImo);
        if (pImo->is_rest())
            m_pRest = static_cast<ImoRest*>(pImo);
        else
            m_pNote = static_cast<ImoNote*>(pImo);
    }

    string generate_source() override
    {
        //@ <!ELEMENT note
        //@     (((grace, %full-note;, (tie, tie?)?) |
        //@      (cue, %full-note;, duration) |
        //@      (%full-note;, duration, (tie, tie?)?)),
        //@      instrument?, %editorial-voice;, type?, dot*,
        //@      accidental?, time-modification?, stem?, notehead?,
        //@      notehead-text?, staff?, beam*, notations*, lyric*, play?)>
        //@ <!ENTITY % full-note "(chord?, (pitch | unpitched | rest))">


        if (m_pRest && m_pRest->is_gap())
        {
            add_forward();
            return m_source.str();
        }


        start_element("note", m_pNR);
        //add_attributes();
        close_start_tag();

        if (m_pNote)
        {
            add_chord();
            add_pitch();
        }
        else
        {
            empty_element("rest");
        }

        add_duration();
        add_ties();
        add_voice();
        add_type();
        add_dots();
        add_accidental();
        add_time_modification();
        add_stem();
        //add_notehead();
        //add_notehead_text();
        add_staff(m_pNR);
        add_beam();
        add_notations();
        //add_lyrics();
        //add_play();

//        source_for_base_staffobj(m_pNR);
        end_element();   //note

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_forward()
    {
        start_element_no_attribs("forward");
        add_duration();
        end_element();  //forward
    }

    //-----------------------------------------------------------------------------------
    void add_chord()
    {
        if (m_pNote->is_in_chord() && !m_pNote->is_start_of_chord())
            empty_element("chord");
    }

    //-----------------------------------------------------------------------------------
    void add_pitch()
    {
        static const string sNoteName[7] = { "C",  "D", "E", "F", "G", "A", "B" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pNR->get_step() == k_no_pitch)
        {
            empty_element("unpitched");
            return;
        }

        start_element_no_attribs("pitch");

        //step
        create_element("step", sNoteName[m_pNR->get_step()] );

        //alter
        float acc = m_pNote->get_actual_accidentals();
        if (acc != 0.0f && acc != k_acc_not_computed)
        {
            create_element("alter", acc);
        }

        //octave
        create_element("octave", sOctave[m_pNR->get_octave()] );

        end_element();  //pitch
    }

    //-----------------------------------------------------------------------------------
    void add_duration()
    {
        TimeUnits divisions = TimeUnits( m_pExporter->get_divisions() );
        int dur = int(round( m_pNR->get_duration() * divisions / TimeUnits(64) ));
        create_element("duration", dur);

        if (m_pRest || !m_pNote->is_in_chord() || m_pNote->is_start_of_chord())
            m_pExporter->increment_current_timepos(dur);
    }

    //-----------------------------------------------------------------------------------
    void add_ties()
    {
        if (m_pNote)
        {
            if (m_pNote->is_tied_prev())
            {
                start_element("tie");
                add_attribute("type", "stop");
                end_element(false, false);
            }
            if (m_pNote->is_tied_next())
            {
                start_element("tie");
                add_attribute("type", "start");
                end_element(false, false);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_voice()
    {
        create_element("voice", m_pNR->get_voice());
    }

    //-----------------------------------------------------------------------------------
    void add_type()
    {
        if (m_pRest && m_pRest->is_full_measure())
            return;     //type is not required for full measure rests

        int type = m_pNR->get_note_type();
        if (type >= 0)
        {
            start_element_no_attribs("type");
            switch(type)
            {
                case k_longa:   m_source << "long";     break;
                case k_breve:   m_source << "breve";    break;
                case k_whole:   m_source << "whole";    break;
                case k_half:    m_source << "half";     break;
                case k_quarter: m_source << "quarter";  break;
                case k_eighth:  m_source << "eighth";   break;
                case k_16th:    m_source << "16th";     break;
                case k_32nd:    m_source << "32nd";     break;
                case k_64th:    m_source << "64th";     break;
                case k_128th:   m_source << "128th";    break;
                case k_256th:   m_source << "256th";    break;
            }
        }
        end_element(k_in_same_line);
    }

    //-----------------------------------------------------------------------------------
    void add_dots()
    {
        for (int dots = m_pNR->get_dots(); dots > 0; --dots)
        {
            empty_element("dot");
        }
    }

    //-----------------------------------------------------------------------------------
    void add_time_modification()
    {
        int top = m_pNR->get_time_modifier_top();
        int bottom = m_pNR->get_time_modifier_bottom();
        if (top != 1 || bottom != 1)
        {
            start_element_no_attribs("time-modification");
            create_element("actual-notes", bottom);
            create_element("normal-notes", top);
            end_element();  //time-modification
        }
    }

    //-----------------------------------------------------------------------------------
    void add_accidental()
    {
        if (m_pNote)
        {
            EAccidentals acc = m_pNote->get_notated_accidentals();
            if (acc != k_no_accidentals)
            {
                string name = MxlExporter::accidentals_to_mxl_name(acc);
                if (!name.empty())
                {
                    start_element_no_attribs("accidental");
                    m_source << name;
                    end_element(k_in_same_line);
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_stem()
    {
        if (m_pNote)
        {
            int stem = m_pNote->get_stem_direction();
            if (stem != k_stem_default)
            {
                start_element_no_attribs("stem");
                switch(stem)
                {
                    case k_stem_up:         m_source << "up";       break;
                    case k_stem_down:       m_source << "down";     break;
                    case k_stem_double:     m_source << "double";   break;
                    case k_stem_none:       m_source << "none";     break;
                }
                end_element(k_in_same_line);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_beam()
    {
        if (m_pNote && m_pNote->is_beamed())
        {
            BeamMxlGenerator gen(m_pNote, m_pExporter);
            m_source << gen.generate_source();
        }
    }

    //-----------------------------------------------------------------------------------
    void add_notations()
    {
        //<tied>
        if (m_pNote)
        {
            if (m_pNote->is_tied_prev())
            {
                start_element_if_not_started("notations");
                start_element("tied");
                add_attribute("type", "stop");
                end_element(false, false);
            }
            if (m_pNote->is_tied_next())
            {
                start_element_if_not_started("notations");
                start_element("tied");
                add_attribute("type", "start");
                end_element(false, false);
            }
        }

        //slur, tuplet
        add_relobj_notations();

        //<glissando> not imported
        //<slide> not imported
        //+<ornaments>
        //+<technical>
        //<dynamics>
        //+<articulations>
        //+<fermata>
        add_auxobj_notations();

        //<arpeggiate>
        //<non-arpeggiate>
        //<accidental-mark>
        //<other-notation>

        end_element_if_started("notations");
    }

    //-----------------------------------------------------------------------------------
    void add_relobj_notations()
    {
        if (m_pNR->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pNR->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (pRO->is_tuplet() )
                {
                    TupletMxlGenerator exporter(pRO, m_pNR, m_pExporter);
                    m_source << exporter.generate_source();
                }
                else if (pRO->is_slur() )
                {
                    SlurMxlGenerator exporter(pRO, m_pNote, m_pExporter);
                    m_source << exporter.generate_source();
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_auxobj_notations()
    {
        ImoAttachments* pAuxObjs = m_pNR->get_attachments();
        if (pAuxObjs == nullptr)
            return;

        //collect notations by types
        list<ImoAuxObj*> articulations;
        list<ImoAuxObj*> dynamics;
        list<ImoAuxObj*> ornaments;
        list<ImoAuxObj*> technical;
        ImoObj::children_iterator it;
        for (it=pAuxObjs->begin(); it != pAuxObjs->end(); ++it)
        {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>(*it);
            if (pAO->is_fermata() )
            {
                start_element_if_not_started("notations");
                FermataMxlGenerator exporter(pAO, m_pExporter);
                m_source << exporter.generate_source();
            }
            else if (pAO->is_articulation() )
                articulations.push_back(pAO);
            else if (pAO->is_dynamics_mark() )
                dynamics.push_back(pAO);
            else if (pAO->is_ornament() )
                ornaments.push_back(pAO);
            else if (pAO->is_technical() )
                technical.push_back(pAO);
        }

        //add notations by types
        if (articulations.size() > 0)
        {
            start_element_if_not_started("notations");
            start_element_no_attribs("articulations");
            for (auto pAO : articulations)
            {
                add_articulation(pAO);
            }
            end_element();  //articulations
        }
        if (dynamics.size() > 0)
        {
            start_element_if_not_started("notations");
            for (auto pAO : dynamics)
            {
                add_dynamics(pAO);
            }
        }
        if (ornaments.size() > 0)
        {
            start_element_if_not_started("notations");
            start_element_no_attribs("ornaments");
            for (auto pAO : ornaments)
            {
                add_ornament(pAO);
            }
            end_element();  //ornaments
        }
        if (technical.size() > 0)
        {
            start_element_if_not_started("notations");
            start_element_no_attribs("technical");
            for (auto pAO : technical)
            {
                add_technical(pAO);
            }
            end_element();  //technical
        }
    }

    //-----------------------------------------------------------------------------------
    void add_articulation(ImoAuxObj* pAO)
    {
        if (pAO->is_articulation_symbol())
        {
            ImoArticulationSymbol* pSymbol = static_cast<ImoArticulationSymbol*>(pAO);
            add_articulation_symbol(pSymbol);
        }
        else
        {
            ImoArticulationLine* pLine = static_cast<ImoArticulationLine*>(pAO);
            add_articulation_line(pLine);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_articulation_symbol(ImoArticulationSymbol* pSymbol)
    {
        string type;
        switch(pSymbol->get_articulation_type())
        {
            //accents
            case k_articulation_accent:             type = "accent";          break;
            case k_articulation_strong_accent:      type = "strong-accent";   break;
            case k_articulation_detached_legato:    type = "detached-legato"; break;
//            case k_articulation_legato_duro:     type = "legato-duro
//            case k_articulation_marccato,				///< Marccato
//            case k_articulation_marccato_legato,			///< Marccato-legato
//            case k_articulation_marccato_staccato,		///< Marccato-staccato
//            case k_articulation_marccato_staccatissimo,	///< Marccato-staccatissimo
//            case k_articulation_mezzo_staccato,			///< Mezzo-staccato
//            case k_articulation_mezzo_staccatissimo,		///< Mezzo-staccatissimo
            case k_articulation_staccato:           type = "staccato";        break;
//            case k_articulation_staccato_duro,			///< Staccato-duro
//            case k_articulation_staccatissimo_duro,		///< Staccatissimo-duro
            case k_articulation_staccatissimo:      type = "staccatissimo";   break;
            case k_articulation_tenuto:             type = "tenuto";          break;

            //stress articulations
            case k_articulation_stress:             type = "stress";          break;
            case k_articulation_unstress:           type = "unstress";        break;

            //other in MusicXML
            case k_articulation_spiccato:           type = "spiccato";        break;

            //breath marks
            case k_articulation_breath_mark:        type = "breath-mark";     break;
            case k_articulation_caesura:            type = "caesura";         break;
            default:
                ;
        }

        if (!type.empty())
        {
            if (pSymbol->get_placement() != k_placement_default)
            {
                start_element(type);
                add_attribute("placement", pSymbol->get_placement() == k_placement_above ?
                                             "above" : "below");
                end_element(false, false);
            }
            else
                empty_element(type);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_articulation_line(ImoArticulationLine* pLine)
    {
        string type;
        switch(pLine->get_articulation_type())
        {
            //jazz pitch articulations
            case k_articulation_scoop:              type = "scoop";           break;
            case k_articulation_plop:               type = "plop";            break;
            case k_articulation_doit:               type = "doit";            break;
            case k_articulation_falloff:            type = "falloff";         break;
            default:
                ;
        }

        if (!type.empty())
        {
            if (pLine->get_placement() != k_placement_default)
            {
                start_element(type);
                add_attribute("placement", pLine->get_placement() == k_placement_above ?
                                             "above" : "below");
                end_element(false, false);
            }
            else
                empty_element(type);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_dynamics(ImoAuxObj* pAO)
    {
        //TODO
        //Dynamics can be either a notation (when attached to ImoNoteRest) or a
        //direction (when attached to ImoDirection)
        //To simplify layout, MusicXML importer changes directions by attachments in some
        //cases and this can be a problem for properly exporting MusicXML. To solve
        //this, perhaps a mark should be added to the modified attachment.

        ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(pAO);

        //AWARE: The placement attribute is used when the dynamics are associated with
        // a <note>. It is ignored when the dynamics are associated with a <direction>
        if (pImo->get_placement() != k_placement_default)
        {
            start_element("dynamics", pImo);
            add_attribute("placement", pImo->get_placement() == k_placement_above ?
                                       "above" : "below");
            close_start_tag();
        }
        else
            start_element_no_attribs("dynamics", pImo);

        m_source << "<" << pImo->get_mark_type() << "/>";
        end_element(k_in_same_line);
    }

    //-----------------------------------------------------------------------------------
    void add_ornament(ImoAuxObj* pAO)
    {
        ImoOrnament* pSymbol = static_cast<ImoOrnament*>(pAO);
        string type;
        switch(pSymbol->get_ornament_type())
        {
            case k_ornament_trill_mark:             type = "trill-mark";            break;
            case k_ornament_vertical_turn:          type = "vertical-turn";         break;
            case k_ornament_shake:                  type = "shake";                 break;
            case k_ornament_turn:                   type = "turn";                  break;
            case k_ornament_delayed_turn:           type = "delayed-turn";          break;
            case k_ornament_inverted_turn:          type = "inverted-turn";         break;
            case k_ornament_delayed_inverted_turn:  type = "delayed-inverted-turn"; break;
            case k_ornament_mordent:                type = "mordent";               break;
            case k_ornament_inverted_mordent:       type = "inverted-mordent";      break;
            case k_ornament_wavy_line:              type = "wavy-line";             break;
            case k_ornament_schleifer:              type = "schleifer";             break;
            case k_ornament_tremolo:                type = "tremolo";               break;
            case k_ornament_other:                  type = "other-ornament";        break;
            default:
                ;
        }

        if (!type.empty())
        {
            if (pSymbol->get_placement() != k_placement_default)
            {
                start_element(type);
                add_attribute("placement", pSymbol->get_placement() == k_placement_above ?
                                             "above" : "below");
                end_element(false, false);
            }
            else
                empty_element(type);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_technical(ImoAuxObj* pAO)
    {
        ImoTechnical* pSymbol = static_cast<ImoTechnical*>(pAO);
        if (pSymbol->get_technical_type() == k_technical_fingering)
            add_technical_fingering(pAO);
        else if (pSymbol->get_technical_type() == k_technical_fret_string)
            add_technical_fret_string(pAO);
        else
        {
            string type;
            switch(pSymbol->get_technical_type())
            {
                case k_technical_up_bow:            type = "up-bow";            break;
                case k_technical_down_bow:          type = "down-bow";          break;
                case k_technical_double_tongue:     type = "double-tongue";     break;
                case k_technical_triple_tongue:     type = "triple-tongue";     break;
                case k_technical_harmonic:          type = "harmonic";          break;
                case k_technical_hole:              type = "hole";              break;
                case k_technical_handbell:          type = "handbell";          break;
                default:
                    ;
            }

            if (!type.empty())
            {
                if (pSymbol->get_placement() != k_placement_default)
                {
                    start_element(type);
                    add_attribute("placement", pSymbol->get_placement() == k_placement_above ?
                                                 "above" : "below");
                    end_element(false, false);
                }
                else
                    empty_element(type);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_technical_fret_string(ImoAuxObj* pAO)
    {
        ImoFretString* pFS = static_cast<ImoFretString*>(pAO);

        start_element_no_attribs("fret");
        m_source << pFS->get_fret();
        end_element(k_in_same_line);

        start_element_no_attribs("string");
        m_source << pFS->get_string();
        end_element(k_in_same_line);
    }

    //-----------------------------------------------------------------------------------
    void add_technical_fingering(ImoAuxObj* pAO)
    {
        ImoFingering* pSymbol = static_cast<ImoFingering*>(pAO);
        const std::list<FingerData>& fingerings = pSymbol->get_fingerings();
        for (auto finger : fingerings)
        {
            start_element_no_attribs("fingering");
            m_source << finger.value;
            end_element(k_in_same_line);
        }
    }

};

//---------------------------------------------------------------------------------------
class PartListMxlGenerator : public MxlGenerator
{
protected:
    ImoScore* m_pObj;

public:
    PartListMxlGenerator(ImoObj* pImo, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
    }

    string generate_source() override
    {
        //<!ELEMENT part-list (part-group*, score-part, (part-group | score-part)*)>
        start_element_no_attribs("part-list", m_pObj);

        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
        {
            start_groups_at(i);
            add_score_part( static_cast<ImoInstrument*>(m_pObj->get_instrument(i)) );
            end_groups_at(i);
        }

        end_element();  //part-list
        return m_source.str();
    }


protected:

    //-----------------------------------------------------------------------------------
    void start_groups_at(int iInstr)
    {
        //@ <!ELEMENT part-group (group-name?, group-name-display?,
        //@           group-abbreviation?, group-abbreviation-display?,
        //@           group-symbol?, group-barline?, group-time?, %editorial;)>

        ImoInstrGroups* pGroups = m_pObj->get_instrument_groups();
        int numGroups = (pGroups ? pGroups->get_num_items() : 0);
        for (int i=0; i < numGroups; ++i)
        {
            ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(pGroups->get_item(i));
            if (pGroup->get_index_to_first_instrument() == iInstr)
            {
                start_element("part-group");
                add_attribute("number", i+1);
                add_attribute("type", "start");
                close_start_tag();

                string name = pGroup->get_name_string();
                if (!name.empty())
                    create_element("group-name", name);

                string abbrev = pGroup->get_abbrev_string();
                if (!abbrev.empty())
                    create_element("group-abbreviation", abbrev);

                string symbol;
                switch (pGroup->get_symbol())
                {
                    case k_group_symbol_brace:      symbol = "brace";  break;
                    case k_group_symbol_bracket:    symbol = "bracket";  break;
                    case k_group_symbol_line:       symbol = "line";  break;
                    default:
                        ;
                }
                if (!symbol.empty())
                    create_element("group-symbol", symbol);


                string barline;
                switch (pGroup->join_barlines())
                {
                    case k_non_joined_barlines:     barline = "no";  break;
                    case k_mensurstrich_barlines:   barline = "Mensurstrich";  break;
                    default:
                        barline = "yes";
                }
                if (!barline.empty())
                    create_element("group-barline", barline);

                end_element();  //part-group
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void end_groups_at(int iInstr)
    {
        ImoInstrGroups* pGroups = m_pObj->get_instrument_groups();
        int numGroups = (pGroups ? pGroups->get_num_items() : 0);
        for (int i=0; i < numGroups; ++i)
        {
            ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(pGroups->get_item(i));
            if (pGroup->get_index_to_last_instrument() == iInstr)
            {
                start_element("part-group");
                add_attribute("number", i+1);
                add_attribute("type", "stop");
                end_element(false, false);  //part-group
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_score_part(ImoInstrument* pInstr)
    {
        //<!ELEMENT score-part (identification?,
        //    part-name, part-name-display?,
        //    part-abbreviation?, part-abbreviation-display?,
        //    group*, score-instrument*,
        //    (midi-device?, midi-instrument?)*)>
        start_element("score-part", pInstr);
        add_attribute("id", pInstr->get_instr_id());
        close_start_tag();

        add_part_name(pInstr->get_name());
        add_part_name_display();
        add_part_abbreviation(pInstr->get_abbrev());
        add_part_abbreviation_display();
        //TODO:
        //    group*, score_instrument*,
        //    (midi_device?, midi_instrument?)*)>

        end_element();  //score-part
    }

    //-----------------------------------------------------------------------------------
    void add_part_name(const TypeTextInfo& name)
    {
        if (!name.text.empty())
            create_element("part-name", name.text);
        else
            empty_element("part-name");
    }

    //-----------------------------------------------------------------------------------
    void add_part_name_display()
    {
        //TODO
    }

    //-----------------------------------------------------------------------------------
    void add_part_abbreviation(const TypeTextInfo& name)
    {
        if (!name.text.empty())
            create_element("part-abbreviation", name.text);
    }

    //-----------------------------------------------------------------------------------
    void add_part_abbreviation_display()
    {
        //TODO
    }

};


//---------------------------------------------------------------------------------------
class ScoreMxlGenerator : public MxlGenerator
{
protected:
    ImoScore* m_pObj;

public:
    ScoreMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
        m_pExporter->set_current_score(m_pObj);

        ColStaffObjs* pTable = m_pObj->get_staffobjs_table();
        if (pTable)
            m_pExporter->save_divisions( pTable->get_divisions() );
    }

    string generate_source() override
    {
        start_element("score-partwise", m_pObj);
        add_attribute("version", "4.0");
        close_start_tag();

        add_identification();
        add_part_list();
        add_parts();
//        add_styles();
//        add_titles();
//        add_page_layout();
//        add_system_layout();
//        add_options();

        end_element();    //score-partwise
        return m_source.str();
    }

protected:

    void add_identification()
    {
        start_element_no_attribs("identification");
        start_element_no_attribs("encoding");
        create_element("encoding-date", m_pExporter->get_export_time());
        create_element("software", "Lomse " + m_pExporter->get_lomse_version());

        //supports
            //no system breaks encoded
        start_element("supports");
        m_source << " type=\"no\" element=\"print\" attribute=\"new-system\" value=\"yes\"";
        end_element(false, false);
            //no page breaks encoded
        start_element("supports");
        m_source << " type=\"no\" element=\"print\" attribute=\"new-page\" value=\"yes\"";
        end_element(false, false);
            //beams are encoded
        start_element("supports");
        m_source << " type=\"yes\" element=\"beam\"";
        end_element(false, false);
            //stems are encoded
        start_element("supports");
        m_source << " type=\"yes\" element=\"stem\"";
        end_element(false, false);
            //accidentals are encoded
        start_element("supports");
        m_source << " type=\"yes\" element=\"accidental\"";
        end_element(false, false);

        end_element();    //encoding
        end_element();    //identification
    }

//    void add_styles()
//    {
////    //styles
////    m_source << m_TextStyles.SourceMNX(1, fUndoData);
//    }
//
//    void add_titles()
//    {
////    //titles and other attached auxobjs
////    if (m_pAuxObjs)
////    {
////	    for (int i=0; i < (int)m_pAuxObjs->size(); i++)
////	    {
////		    m_source << (*m_pAuxObjs)[i]->SourceMNX(1, fUndoData);
////	    }
////    }
//    }
//
//    void add_page_layout()
//    {
////    //first section layout info
////    //TODO: sections
////    //int nSection = 0;
////    {
////        //page layout info
////#if 0
////		std::list<lmPageInfo*>::iterator it;
////		for (it = m_PagesInfo.begin(); it != m_PagesInfo.end(); ++it)
////            m_source << (*it)->SourceMNX(1, fUndoData);
////#else
////        lmPageInfo* pPageInfo = m_PagesInfo.front();
////        m_source << pPageInfo->SourceMNX(1, fUndoData);
////#endif
////        //first system and other systems layout info
////        m_source << m_SystemsInfo.front()->SourceMNX(1, true, fUndoData);
////        m_source << m_SystemsInfo.back()->SourceMNX(1, false, fUndoData);
////    }
//    }
//
//    void add_system_layout()
//    {
//    }

    void add_part_list()
    {
        PartListMxlGenerator exporter(m_pObj, m_pExporter);
        m_source << exporter.generate_source();
    }

    void add_parts()
    {
        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
        {
            add_source_for(m_pObj->get_instrument(i));
        }
    }

};


//---------------------------------------------------------------------------------------
class ScoreObjMxlGenerator : public MxlGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjMxlGenerator(ImoObj* pImo, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    string generate_source() override
    {
//        add_visible();
//        add_color();
//        source_for_base_contentobj(m_pObj);
        return m_source.str();
    }

protected:

//    void add_visible()
//    {
//        if (!m_pObj->is_visible())
//            m_source << " (visible no)";
//    }
//
//    void add_color()
//    {
//        //color (if not black)
//        Color color = m_pObj->get_color();
//        if (color.r != 0 || color.g != 0  || color.b != 0 || color.a != 255)
//            m_source << " (color " << MxlExporter::color_to_mnx(color) << ")";
//    }
};


//---------------------------------------------------------------------------------------
class DirectionMxlGenerator : public MxlGenerator
{
protected:
    ImoDirection* m_pObj;

public:
    DirectionMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDirection*>(pImo);
    }

    string generate_source() override
    {
        //<!ELEMENT direction (direction-type+, offset?,
        //    %editorial-voice;, staff?, sound?)>
        //<!ATTLIST direction
        //    %placement;
        //    %directive;
        //>

        if (m_pObj->get_placement() != k_placement_default)
        {
            start_element("direction", m_pObj);
            add_attribute("placement", m_pObj->get_placement() == k_placement_above ?
                                       "above" : "below");
            close_start_tag();
        }
        else
        {
            start_element_no_attribs("direction", m_pObj);
        }
        add_direction_type();
        end_element();

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_direction_type()
    {
        add_auxobj_directions();
        add_relobj_directions();
    }

    //-----------------------------------------------------------------------------------
    void add_auxobj_directions()
    {
        ImoAttachments* pAuxObjs = m_pObj->get_attachments();
        if (pAuxObjs == nullptr)
            return;

        ImoObj::children_iterator it;
        for (it=pAuxObjs->begin(); it != pAuxObjs->end(); ++it)
        {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>(*it);

            if (pAO->is_symbol_repetition_mark() )
                add_coda_segno( static_cast<ImoSymbolRepetitionMark*>(pAO) );

            else if (pAO->is_score_text() || pAO->is_text_repetition_mark())
                add_words( static_cast<ImoScoreText*>(pAO) );

            else if (pAO->is_dynamics_mark() )
                add_dynamics( static_cast<ImoDynamicsMark*>(pAO) );

            else if (pAO->is_metronome_mark() )
                add_metronome( static_cast<ImoMetronomeMark*>(pAO) );

            else if (pAO->is_pedal_mark() )
                add_pedal_mark( static_cast<ImoPedalMark*>(pAO) );

            else
            {
                //TODO
                LOMSE_LOG_ERROR("Direction not supported in MusicXL exporter");
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_coda_segno(ImoSymbolRepetitionMark* pImo)
    {
        start_element_no_attribs("direction-type");
        if (pImo->get_symbol() == ImoSymbolRepetitionMark::k_coda)
            empty_element("coda");
        else
            empty_element("segno");

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_words(ImoScoreText* pImo)
    {
        //<!ELEMENT words (#PCDATA)>
        //<!ATTLIST words
        //    %text-formatting;
        //>

        start_element_no_attribs("direction-type");

        start_element_no_attribs("words", pImo);
        m_source << pImo->get_text();
        end_element(k_in_same_line);

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_dynamics(ImoDynamicsMark* pImo)
    {
        start_element_no_attribs("direction-type");

        start_element_no_attribs("dynamics", pImo);
        m_source << "<" << pImo->get_mark_type() << "/>";
        end_element(k_in_same_line);

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_metronome(ImoMetronomeMark* pImo)
    {
        //@ <!ELEMENT metronome
        //@ 	((beat-unit, beat-unit-dot*,
        //@      (per-minute | (beat-unit, beat-unit-dot*))) |
        //@ 	(metronome-note+, (metronome-relation, metronome-note+)?))>
        //@ <!ATTLIST metronome
        //@     %print-style;
        //@     parentheses %yes-no; #IMPLIED
        //@ >

        start_element_no_attribs("direction-type");
        start_element_no_attribs("metronome");
        if (pImo->get_mark_type() == ImoMetronomeMark::k_note_value)
        {
            create_element("beat-unit",
                           MxlExporter::note_type_to_mxl_name(pImo->get_left_note_type()));
            for (int i=pImo->get_left_dots(); i > 0; --i)
                empty_element("beat-unit-dot");
            create_element("per-minute", pImo->get_ticks_per_minute());
        }
        else if (pImo->get_mark_type() == ImoMetronomeMark::k_note_note)
        {
            create_element("beat-unit",
                           MxlExporter::note_type_to_mxl_name(pImo->get_left_note_type()));
            for (int i=pImo->get_left_dots(); i > 0; --i)
                empty_element("beat-unit-dot");

            create_element("beat-unit",
                           MxlExporter::note_type_to_mxl_name(pImo->get_right_note_type()));
            for (int i=pImo->get_right_dots(); i > 0; --i)
                empty_element("beat-unit-dot");
        }
        else
        {
            create_element("per-minute", pImo->get_ticks_per_minute());
        }
        end_element();  //metronome
        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_pedal_mark(ImoPedalMark* pImo)
    {
    }

    //-----------------------------------------------------------------------------------
    void add_relobj_directions()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (pRO->is_wedge() )
                {
                    add_wedge(static_cast<ImoWedge*>(pRO));
                }
                else if (pRO->is_pedal_line() )
                {
                    //TODO
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_wedge(ImoWedge* pImo)
    {
        start_element_no_attribs("direction-type");

        start_element("wedge", pImo);

        if (pImo->get_start_object() == m_pObj)
        {
            add_attribute("type", pImo->is_crescendo() ? "crescendo" : "diminuendo");
            if (pImo->is_niente())
                add_attribute("niente", "yes");
        }
        else
        {
            add_attribute("type", "stop");
            if (pImo->is_niente())
                add_attribute("niente", "yes");
        }

        end_element(false, false);    //wedge

        end_element();  //direction-type
    }

};


//---------------------------------------------------------------------------------------
class StaffObjMxlGenerator : public MxlGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    string generate_source() override
    {
//        add_staff_num();
//        source_for_base_scoreobj(m_pObj);
        return m_source.str();
    }

protected:

//    void add_staff_num()
//    {
//        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
//            && !m_pObj->is_time_signature()
//            && !m_pObj->is_barline() )
//        {
////            m_source << " p" << (m_pObj->get_staff() + 1);
//            //m_source << "<staff>" << (m_pObj->get_staff() + 1) << "</staff>";
//            create_element("staff", m_pObj->get_staff() + 1);
//        }
//    }

};


//---------------------------------------------------------------------------------------
class StylesMxlGenerator : public MxlGenerator
{
protected:
    ImoStyles* m_pObj;

public:
    StylesMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStyles*>(pImo);
    }

    string generate_source() override
    {
//        if (there_is_any_non_default_style())
//        {
//            start_element_no_attribs("styles", m_pObj);
//            add_styles();
//            end_element();
//            empty_line();
//            return m_source.str();
//        }
        return "";
    }

protected:

//    bool there_is_any_non_default_style()
//    {
//     	map<std::string, ImoStyle*>& styles = m_pObj->get_styles_collection();
//
//        map<std::string, ImoStyle*>::iterator it;
//        for(it = styles.begin(); it != styles.end(); ++it)
//        {
//            ImoStyle* pStyle = it->second;
//            if (!pStyle->is_default_style_with_default_values())
//                return true;
//        }
//        return false;
//    }
//
//    void add_styles()
//    {
//     	map<std::string, ImoStyle*>& styles = m_pObj->get_styles_collection();
//
//        map<std::string, ImoStyle*>::iterator it;
//        for(it = styles.begin(); it != styles.end(); ++it)
//        {
//            ImoStyle* pStyle = it->second;
//            if (!pStyle->is_default_style_with_default_values())
//                add_source_for(pStyle);
//        }
//    }

};


//---------------------------------------------------------------------------------------
class TimeSignatureMxlGenerator : public MxlGenerator
{
protected:
    ImoTimeSignature* m_pObj;

public:
    TimeSignatureMxlGenerator(ImoObj* pImo, MxlExporter* pExporter) : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoTimeSignature*>(pImo);
    }

    string generate_source() override
    {
        //@ <!ELEMENT time ((beats, beat-type)+ | senza-misura)>
        //@ <!ATTLIST time
        //@         symbol (common | cut | single-number | normal) #IMPLIED
        //@ >

        start_element("time", m_pObj);
        add_symbol();
        close_start_tag();
        add_beats_type();

        end_element();
        return m_source.str();
    }

protected:

    void add_symbol()
    {
        int type = m_pObj->get_type();
        if (type != ImoTimeSignature::k_normal)
        {
            if (type == ImoTimeSignature::k_common)
                add_attribute("symbol", "common");
            else if (type == ImoTimeSignature::k_cut)
                add_attribute("symbol", "cut");
            else if (type == ImoTimeSignature::k_single_number)
                add_attribute("symbol", "single-number");
        }
    }

    void add_beats_type()
    {
//        int type = m_pObj->get_type();
//    enum { k_normal=0, k_common, k_cut, k_single_number, k_senza_misura, };
//        if (type == ImoTimeSignature::k_normal)
//        {
            create_element("beats", m_pObj->get_top_number());
            create_element("beat-type", m_pObj->get_bottom_number());
//        }
//        else
//        {
//        }
    }

};



//=======================================================================================
// MxlGenerator implementation
//=======================================================================================
MxlGenerator::MxlGenerator(MxlExporter* pExporter)
    : m_pExporter(pExporter)
{
}

//---------------------------------------------------------------------------------------
void MxlGenerator::start_element(const string& name, ImoObj* pImo)
{
    new_line_and_indent_spaces();
    m_source << "<" << name;
    m_pExporter->push_tag(name);
//    if (pImo && m_pExporter->get_add_id())
//        m_source << " id=\"" << std::dec << pImo->get_id() << "\"";
    increment_indent();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::start_element_no_attribs(const string& name, ImoObj* pImo)
{
    start_element(name, pImo);
    close_start_tag();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::empty_element(const string& name, ImoObj* pImo)
{
    start_element(name, pImo);
    end_element(false, false);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::create_element(const string& name, const string& content)
{
    start_element_no_attribs(name);
    m_source << content;
    end_element(k_in_same_line);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::create_element(const string& name, int content)
{
    start_element_no_attribs(name);
    m_source << content;
    end_element(k_in_same_line);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::close_start_tag()
{
    m_source << ">";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::start_attrib(const string& name)
{
    m_source << " " << name << "=\"";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::end_attrib()
{
    m_source << "\"";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attribute(const string& name, const string& value)
{
    start_attrib(name);
    m_source << value;
    end_attrib();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attribute(const string& name, int value)
{
    start_attrib(name);
    m_source << value;
    end_attrib();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::end_element(bool fInNewLine, bool fNewElement)
{
    if (fNewElement)
    {
        decrement_indent();
        if (fInNewLine)
            new_line_and_indent_spaces();
        if (!m_pExporter->are_there_open_tags())
            m_source << "Error in logic. Requested to close element and no more elements!"
                << endl;
        else
            m_source << "</" << m_pExporter->current_open_tag() << ">";
    }
    else
    {
        m_source << "/>";
        decrement_indent();
    }

    if (m_pExporter->are_there_open_tags())
        m_pExporter->pop_tag();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_separator_line()
{
    if (!m_pExporter->get_remove_separator_lines())
    {
        new_line_and_indent_spaces();
        m_source << "<!--=========================================================-->";
    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::start_comment()
{
    new_line_and_indent_spaces();
    m_source << "<!-- ";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::end_comment()
{
    m_source << " -->";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::empty_line()
{
    m_source.clear();
    new_line();
}
//---------------------------------------------------------------------------------------
void MxlGenerator::new_line_and_indent_spaces(bool fStartLine)
{
    m_source.clear();
    if (!m_pExporter->get_remove_newlines())
    {
        if (fStartLine)
            new_line();
        int indent = m_pExporter->get_indent() * k_indent_step;
        while (indent > 0)
        {
            m_source << " ";
            indent--;
        }
    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::new_line()
{
    if (!m_pExporter->get_remove_newlines())
        m_source << endl;
}

//---------------------------------------------------------------------------------------
void MxlGenerator::start_element_if_not_started(const string& tag)
{
    if (m_pExporter->current_open_tag() != tag)
        start_element_no_attribs(tag);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::end_element_if_started(const string& tag)
{
    if (m_pExporter->current_open_tag() == tag)
        end_element();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_source_for(ImoObj* pImo)
{
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjMxlGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjMxlGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjMxlGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_base_imobj(ImoObj* pImo)
{
    increment_indent();
    ImObjMxlGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
    decrement_indent();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::source_for_inline_level_object(ImoInlineLevelObj* pImo,
                                                  bool UNUSED(fFirstItem),
                                                  ImoStyle* pParentStyle)
{
    //TODO: source code for these objects. This is just an skeleton

    //composite content objects
    if (pImo->is_text_item())
    {
        ImoTextItem* pText = static_cast<ImoTextItem*>(pImo);
        ImoStyle* pStyle = pText->get_style();
        if (pStyle && pStyle != pParentStyle)
        {
            start_element("txt", pImo);
            if (pStyle && pStyle != pParentStyle)
                add_optional_style(pText);
            close_start_tag();
            m_source << pText->get_text();
            end_element();
        }
        else
            m_source << pText->get_text();
    }
//    else if (pImo->is_box_inline())
//    {
//        add_source_for(pImo);
////        //ImoBoxInline* pIB = static_cast<ImoBoxInline*>(pImo);
////        m_source << " (box_inline: " << pImo->get_name() << ")";
//    }
//
//    //atomic content objects
    else
    {
        add_source_for(pImo);
        //m_source << " (atomic content: " << pImo->get_name() << ")";
    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_optional_style(ImoContentObj* pObj)
{
    ImoStyle* pStyle = pObj->get_style();
    if (pStyle && !pStyle->is_default_style_with_default_values())
        m_source << " style=\"" << pStyle->get_name() << "\"";
}

//---------------------------------------------------------------------------------------
void MxlGenerator::increment_indent()
{
    m_pExporter->increment_indent();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::decrement_indent()
{
    m_pExporter->decrement_indent();
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_staff(ImoStaffObj* pSO)
{
    ImoInstrument* pInsr = m_pExporter->get_current_instrument();
    if (pInsr && pInsr->get_num_staves() > 1)
    {
        create_element("staff", pSO->get_staff() + 1);
    }
}


//=======================================================================================
// MxlExporter implementation
//=======================================================================================
MxlExporter::MxlExporter(LibraryScope& libScope)
    : m_libraryScope(libScope)
    , m_pLeftBarline(LOMSE_NEW BarlineData)
{
    m_lomseVersion = libScope.get_version_string();
    time_t time_sec = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm time_tm;

#if (LOMSE_COMPILER_MSVC == 1)
    localtime_s(&time_tm, &time_sec);
#else
    localtime_r(&time_sec, &time_tm);
#endif

    stringstream ss;

    // we could do 'ss << std::put_time(&time_tm, "%Y-%b-%d")' but it's not available in GCC until 5.0
    // using 'strftime' instead

    vector<char> buf(100);
    strftime(buf.data(), buf.size(), "%Y-%m-%d", &time_tm);
    ss << buf.data();

    m_exportTime = ss.str();

    //other initializations
    m_slurs.fill(k_no_imoid);
    m_tuplets.fill(k_no_imoid);
}

//---------------------------------------------------------------------------------------
MxlExporter::~MxlExporter()
{
    delete m_pLeftBarline;
}

//---------------------------------------------------------------------------------------
string MxlExporter::get_version_and_time_string() const
{
    string value = m_lomseVersion;
    value.append(", date ");
    value.append(m_exportTime);
    return value;
}

//---------------------------------------------------------------------------------------
string MxlExporter::get_source(ImoObj* pImo)
{
    MxlGenerator* pGen = new_generator(pImo);
    string source = pGen->generate_source();
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
string MxlExporter::get_source(AScore score)
{
    if (score.is_valid())
        return get_source(score.internal_object());

    return string();
}

//---------------------------------------------------------------------------------------
MxlGenerator* MxlExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_barline:         return LOMSE_NEW BarlineMxlGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefMxlGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocMxlGenerator(pImo, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentMxlGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureMxlGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataMxlGenerator(pImo, this);
        case k_imo_note_regular:    return LOMSE_NEW NoteRestMxlGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW NoteRestMxlGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreMxlGenerator(pImo, this);
        case k_imo_direction:       return LOMSE_NEW DirectionMxlGenerator(pImo, this);
        case k_imo_style:           return LOMSE_NEW DefineStyleMxlGenerator(pImo, this);
        case k_imo_styles:          return LOMSE_NEW StylesMxlGenerator(pImo, this);
        case k_imo_time_signature:  return LOMSE_NEW TimeSignatureMxlGenerator(pImo, this);

        default:
            return new ErrorMxlGenerator(pImo, this);
    }
}

//---------------------------------------------------------------------------------------
void MxlExporter::save_data_for_left_barline(BarlineData& data)
{
    *m_pLeftBarline = data;
}

//---------------------------------------------------------------------------------------
void MxlExporter::clear_data_for_left_barline()
{
    m_pLeftBarline->style = "";
}

//---------------------------------------------------------------------------------------
const BarlineData& MxlExporter::get_data_for_left_barline() const
{
    return *m_pLeftBarline;
}

//---------------------------------------------------------------------------------------
bool MxlExporter::left_barline_required()
{
    return m_pLeftBarline && !m_pLeftBarline->style.empty();
}

//---------------------------------------------------------------------------------------
int MxlExporter::start_slur_and_get_number(ImoId slurId)
{
    for (int i=0; i < k_max_slur_number; ++i)
    {
        if (m_slurs[i] == k_no_imoid)
        {
            m_slurs[i] = slurId;
            return i+1;
        }
    }

    LOMSE_LOG_ERROR("more than 16 open slurs!");
    return 1;
}

//---------------------------------------------------------------------------------------
int MxlExporter::close_slur_and_get_number(ImoId slurId)
{
    for (int i=0; i < k_max_slur_number; ++i)
    {
        if (m_slurs[i] == slurId)
        {
            m_slurs[i] = k_no_imoid;
            return i+1;
        }
    }

    LOMSE_LOG_ERROR("slurId is not yet created!");
    return 1;
}

//---------------------------------------------------------------------------------------
int MxlExporter::start_tuplet_and_get_number(ImoId tupletId)
{
    for (int i=0; i < k_max_tuplet_number; ++i)
    {
        if (m_tuplets[i] == k_no_imoid)
        {
            m_tuplets[i] = tupletId;
            return i+1;
        }
    }

    LOMSE_LOG_ERROR("more than 16 open tuplets!");
    return 1;
}

//---------------------------------------------------------------------------------------
int MxlExporter::close_tuplet_and_get_number(ImoId tupletId)
{
    for (int i=0; i < k_max_tuplet_number; ++i)
    {
        if (m_tuplets[i] == tupletId)
        {
            m_tuplets[i] = k_no_imoid;
            return i+1;
        }
    }

    LOMSE_LOG_ERROR("tupletId is not yet created!");
    return 1;
}

// static methods

//---------------------------------------------------------------------------------------
string MxlExporter::barline_type_to_mnx(int barType)
{
    //AWARE: indexes in correspondence with enum in ImoBarline
    static const string name[] = {
        "none",
        "simple",
        "double",
        "start",
        "end",
        "endRepetition",
        "startRepetition",
        "doubleRepetition",
    };

    return name[barType];
}

//---------------------------------------------------------------------------------------
string MxlExporter::note_type_to_mxl_name(int noteType)
{
    switch(noteType)
    {
        case k_longa:   return "longa";
        case k_breve:   return "breve";
        case k_whole:   return "whole";
        case k_half:    return "half";
        case k_quarter: return "quarter";
        case k_eighth:  return "eighth";
        case k_16th:    return "16th";
        case k_32nd:    return "32nd";
        case k_64th:    return "64th";
        case k_128th:   return "128th";
        case k_256th:   return "256th";
        default:
        {
            stringstream s;
            s << "Invalid noteType. Value=" << noteType;
            LOMSE_LOG_ERROR(s.str());
            return "quarter";
        }
    }
}

//---------------------------------------------------------------------------------------
string MxlExporter::accidentals_to_mxl_name(int acc)
{
    switch(acc)
    {
        //standard accidentals
        case k_natural:             return "natural";
        case k_sharp:               return "sharp";
        case k_flat:                return "flat";
        case k_flat_flat:           return "flat-flat";
        case k_double_sharp:        return "double-sharp";
        case k_natural_flat:        return "natural-flat";
        case k_natural_sharp:       return "natural-sharp";
        case k_sharp_sharp:         return "sharp-sharp";
        case k_acc_triple_sharp:    return "triple-sharp";
        case k_acc_triple_flat:     return "triple-flat";

        //microtonal accidentals: Tartini-style quarter-tone accidentals
        case k_acc_quarter_flat:            return "quarter-flat";
        case k_acc_quarter_sharp:           return "quarter-sharp";
        case k_acc_three_quarters_flat:     return "three-quarters-flat";
        case k_acc_three_quarters_sharp:    return "three-quarters-sharp";

        //microtonal accidentals: quarter-tone accidentals that include arrows pointing down or up
        case k_acc_sharp_down:          return "sharp-down";
        case k_acc_sharp_up:            return "sharp-up";
        case k_acc_natural_down:        return "natural-down";
        case k_acc_natural_up:          return "natural-up";
        case k_acc_flat_down:           return "flat-down";
        case k_acc_flat_up:             return "flat-up";
        case k_acc_double_sharp_down:   return "double-sharp-down";
        case k_acc_double_sharp_up:     return "double-sharp-up";
        case k_acc_flat_flat_down:      return "flat-flat-down";
        case k_acc_flat_flat_up:        return "flat-flat-up";
        case k_acc_arrow_down:          return "arrow-down";
        case k_acc_arrow_up:            return "arrow-up";

        //accidentals used in Turkish classical music
        case k_acc_slash_quarter_sharp: return "slash-quarter-sharp";
        case k_acc_slash_sharp:         return "slash-sharp";
        case k_acc_slash_flat:          return "slash-flat";
        case k_acc_double_slash_flat:   return "double-slash-flat";

    	//superscripted versions of standard accidental signs, used in Turkish folk music
        case k_acc_sharp_1:     return "sharp-1";
        case k_acc_sharp_2:     return "sharp-2";
        case k_acc_sharp_3:     return "sharp-3";
        case k_acc_sharp_5:     return "sharp-5";
        case k_acc_flat_1:      return "flat-1";
        case k_acc_flat_2:      return "flat-2";
        case k_acc_flat_3:      return "flat-3";
        case k_acc_flat_4:      return "flat-4";

        //microtonal flat and sharp accidentals used in Iranian and Persian music
        case k_acc_sori:    return "sori";
        case k_acc_koron:   return "koron";

	    //other, usually used in combination with the SMuFl glyph
        case k_acc_other:   return "other";

        default:
        {
            LOMSE_LOG_ERROR("Program maintenance error?: Missing case for accidental %d.",
                            acc);
            return "";
        }
    }
}

//---------------------------------------------------------------------------------------
string MxlExporter::color_to_mnx(Color color)
{
    stringstream source;
    source << "#";
    int r = color.r;
    int g = color.g;
    int b = color.b;
    int a = color.a;
    source << std::hex << setfill('0') << setw(2) << r;
    source << std::hex << setfill('0') << setw(2) << g;
    source << std::hex << setfill('0') << setw(2) << b;
    source << std::hex << setfill('0') << setw(2) << a;
    return source.str();
}

//---------------------------------------------------------------------------------------
string MxlExporter::float_to_string(float UNUSED(num))
{
    return "(TODO: float_to_string)";
}


}  //namespace lomse
