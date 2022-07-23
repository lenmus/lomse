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
#include "lomse_ldp_exporter.h"
#include "lomse_logger.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"
#include "lomse_im_attributes.h"


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
    bool fRepeat = false;
    bool fEnding = false;
    ImoVoltaBracket* pVoltaBracket = nullptr;   //start volta for left, or end volta for right
    ImoBarline* pVoltaParent = nullptr;
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
    void create_element(const string& name, float content);
    void close_start_tag();
    void start_attrib(const string& name);
    void end_attrib();
    void add_attribute(const string& name, const string& value);
    void add_attribute(const string& name, int value);
    void add_attribute(const string& name, float value);
    void end_element(bool fInNewLine = true, bool fNewElement=true);
    void add_separator_line();
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void new_line();
    void start_element_if_not_started(const std::string& tag);
    void end_element_if_started(const std::string& tag);
    void add_source_for(ImoObj* pImo, ImoObj* pParent=nullptr);
    void increment_indent();
    void decrement_indent();

    void add_optional_style(ImoContentObj* pObj);

    void add_staff(ImoStaffObj* pSO);

    //generators for common attributes
    void add_attributes_for_print_style_align(ImoObj* pImo);
    void add_attributes_for_print_style(ImoObj* pImo);
    void add_attributes_for_printout(ImoObj* pImo);
    void add_attributes_for_position(ImoObj* pObj);
    void add_attibutes_for_text_formatting(ImoObj* pObj);
    void add_attributes_for_font(ImoObj* pObj);
    void add_attribute_color(ImoObj* pImo);
    void add_attribute_print_object(ImoObj* pObj);
    void add_attribute_optional_unique_id(ImoObj* pObj);


};

const bool k_in_same_line = false;
//const bool k_in_new_line = true;
const int k_indent_step = 3;

//=======================================================================================
// generators for specific elements
//=======================================================================================

//---------------------------------------------------------------------------------------
class ArpeggioMxlGenerator : public MxlGenerator
{
protected:
    ImoArpeggio* m_pImo;
    ImoNote* m_pNote;

public:
    ArpeggioMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoArpeggio*>(pImo) )
        , m_pNote( static_cast<ImoNote*>(pParent) )
    {
    }

    string generate_source() override
    {
//        if (m_pNote == m_pImo->get_start_object())
//        {
            start_element_if_not_started("notations");
            start_element("arpeggiate");

            //TODO
            //attrib: number %number-level; #IMPLIED

            //attrib: direction
            if (m_pImo->get_type() != k_arpeggio_standard)
                add_attribute("direction", m_pImo->get_type() == k_arpeggio_arrow_up ? "up" : "down") ;

            //attrib: unbroken %yes-no; #IMPLIED

            add_attributes_for_position(m_pImo);
            //%placement;
            //%color;
            add_attribute_optional_unique_id(m_pImo);

            end_element(false, false);  //arpeggiate
//        }
//        else if (m_pNote == m_pImo->get_end_object())
//        {
//            start_element_if_not_started("notations");
//            empty_element("arpeggiate");
//        }
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class BarlineMxlGenerator : public MxlGenerator
{
protected:
    ImoBarline*  m_pObj;
    BarlineData  m_right;       //data for right barline
    BarlineData  m_left;        //data for left barline in next measure

public:
    BarlineMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    string generate_source() override
    {
        //When an ImoBarline is processed it must be split into data for that barline
        //(right data) and data for a possible barline at start of next measure (left data)

        split_barline();
        determine_volta_brackets();
        generate_source_for_barline(m_right);
        save_data_for_left_barline();

        return m_source.str();
    }

    string generate_left_barline()
    {
        //When starting a measure this method is invoked to add, if necessary, a left
        //barline

        BarlineData data = m_pExporter->get_data_for_left_barline();
        m_pExporter->clear_data_for_left_barline();
        return generate_source_for_barline(data);
    }


protected:

    //-----------------------------------------------------------------------------------
    void split_barline()
    {
        m_left.fRepeat = false;
        m_left.times = 0;
        m_left.location = "left";

        m_right.fRepeat = false;
        m_right.times = m_pObj->get_num_repeats();
        m_right.location = m_pObj->is_middle() ? "middle" : "right";
        m_right.winged = m_pObj->get_winged();

        if (!m_pObj->is_visible())
        {
            m_right.style = "none";
            return;
        }

        switch (m_pObj->get_type())
        {
            case k_barline_simple:
                m_right.style = "";
                return;

            case k_barline_none:
                m_right.style = "none";
                return;

            case k_barline_double:
                m_right.style = "light-light";
                return;

            case k_barline_start:
                m_right.style = "heavy-light";
                return;

            case k_barline_end:
                m_right.style = "light-heavy";
                return;

            case k_barline_start_repetition:
                m_right.style = "";
                m_left.style = "heavy-light";
                m_left.fRepeat = true;
                m_left.location = "left";
                //TODO: in current IM times is only save on end of repetition barline.
                //Need to find next barline to get this information !
                m_left.times = 1;
                m_left.winged = m_right.winged;
                return;

            case k_barline_end_repetition:
                m_right.fRepeat = true;
                m_right.style = "light-heavy";
                return;

            case k_barline_double_repetition:
                m_right.style = "light-heavy";
                m_right.fRepeat = true;
                m_left.style = "heavy-light";
                m_left.fRepeat = true;
                m_left.location = "left";
                //TODO: in current IM times is only save on end of repetition barline.
                //Need to find next barline to get this information !
                m_left.times = 1;
                m_left.winged = m_right.winged;
                return;

            case k_barline_double_repetition_alt:
                m_right.style = "heavy";
                m_right.fRepeat = true;
                m_left.style = "heavy";
                m_left.fRepeat = true;
                m_left.location = "left";
                //TODO: in current IM times is only save on end of repetition barline.
                //Need to find next barline to get this information !
                m_left.times = 1;
                m_left.winged = m_right.winged;
                return;

            case k_barline_dashed:
                m_right.style = "dashed";
                return;

            case k_barline_dotted:
                m_right.style = "dotted";
                return;

            case k_barline_heavy:
                m_right.style = "heavy";
                return;

            case k_barline_heavy_heavy:
                m_right.style = "heavy-heavy";
                return;

            case k_barline_short:
                m_right.style = "short";
                return;

            case k_barline_tick:
                m_right.style = "tick";
                return;

            default:
            {
                stringstream msg;
                msg << "Program maintenace error? Barline " << m_pObj->get_type()
                    << " (" << LdpExporter::barline_type_to_ldp(m_pObj->get_type())
                    << ") not included in switch statement. Replaced by default value.";
                LOMSE_LOG_ERROR(msg.str());
                m_right.style = "";
                return;
            }
        }
    }

    //-----------------------------------------------------------------------------------
    string generate_source_for_barline(const BarlineData& data)
    {
        if (data.style.empty() && data.times == 0 && data.pVoltaBracket == nullptr)
            return "";

        start_element("barline", m_pObj);
        add_attributes(data);
        close_start_tag();

        //bar-style?
        if (!data.style.empty() && !(data.style == "none" && data.pVoltaBracket != nullptr))
            create_element("bar-style", data.style);

        //TODO: Not yet imported children
        //%editorial;
        //wavy-line?,
        //segno?
        //coda?
        //(fermata, fermata?)?

        //ending
        add_ending(data);

        //repeat
        add_repeat(data);

        end_element();  //barline

        return m_source.str();
    }

    //-----------------------------------------------------------------------------------
    void add_attributes(const BarlineData& data)
    {
        //attrib: location
        if (data.location != "right")
            add_attribute("location", data.location);

        //TODO
        //attrib: segno CDATA #IMPLIED      <-- Not yet imported
        //attrib: coda CDATA #IMPLIED       <-- Not yet imported
        //attrib: divisions CDATA #IMPLIED  <-- Not yet imported
    }

    //-----------------------------------------------------------------------------------
    void determine_volta_brackets()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (pRO->is_volta_bracket() )
                {
                    ImoVoltaBracket* pVolta = static_cast<ImoVoltaBracket*>(pRO);
                    if (pVolta->get_start_barline() == m_pObj)
                    {
                        m_left.pVoltaBracket = pVolta;
                        m_left.pVoltaParent = m_pObj;
                    }
                    else
                    {
                        m_right.pVoltaBracket = pVolta;
                        m_right.pVoltaParent = m_pObj;
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void save_data_for_left_barline()
    {
        m_pExporter->save_data_for_left_barline(m_left);
    }

    //-----------------------------------------------------------------------------------
    void add_repeat(const BarlineData& data)
    {
        if (!data.fRepeat)
            return;

        start_element("repeat");

        //attrib: direction
        add_attribute("direction", data.location == "left" ? "forward" : "backward");

        //attrib: times
        if (data.times > 1)
            add_attribute("times", data.times);

        //attrib: winged
        string w;
        switch(data.winged)
        {
            case k_wings_none:              w="none";               break;
            case k_wings_straight:          w="straight";           break;
            case k_wings_curved:            w="curved";             break;
            case k_wings_double_straight:   w="double-straight";    break;
            case k_wings_double_curved:     w="double-curved";      break;
        }
        if (data.winged != k_wings_none)
            add_attribute("winged", w);

        end_element(false, false);
    }

    //-----------------------------------------------------------------------------------
    void add_ending(const BarlineData& data)
    {
        if (data.pVoltaBracket)
            add_source_for(data.pVoltaBracket, data.pVoltaParent);
    }

};


//---------------------------------------------------------------------------------------
class BeamMxlGenerator : public MxlGenerator
{
protected:
    ImoNoteRest* m_pNR;
    ImoBeam* m_pBeam;

public:
    BeamMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
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
        //TODO
        //<!ATTLIST beam
        //    number %beam-level; "1"
        //    repeater %yes-no; #IMPLIED
        //    fan (accel | rit | none) #IMPLIED
        //    %color;
        add_attribute_optional_unique_id(m_pBeam);
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
    ClefMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    string generate_source() override
    {
        start_element_if_not_started("attributes");
        start_element("clef", m_pObj);
        add_staff_number();
        //TODO
        //attrb: additional
        //attrb: size
        //attrb: after-barline
        add_attributes_for_print_style(m_pObj);
        add_attribute_print_object(m_pObj);
        close_start_tag();

        add_sign_and_line();

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
class DefineStyleMxlGenerator : public MxlGenerator
{
protected:
    ImoStyle* m_pObj;

public:
    DefineStyleMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
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
class DirectionMxlGenerator : public MxlGenerator
{
protected:
    ImoDirection* m_pObj;

public:
    DirectionMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDirection*>(pImo);
    }

    string generate_source() override
    {

        if (!is_empty_direction())
        {
            end_element_if_started("attributes");

            //attrib: %placement;
            bool fHasAttributes = false;
            if (m_pObj->get_placement() != k_placement_default)
            {
                start_element("direction", m_pObj);
                add_attribute("placement", m_pObj->get_placement() == k_placement_above ?
                                           "above" : "below");
                fHasAttributes = true;
            }

            //TODO attrib: %directive;
//            if (m_pObj->???????????)
//            {
//                start_element_if_not_started("direction");
//                add_attribute("", "");
//                fHasAttributes = true;
//            }

            if (fHasAttributes)
                close_start_tag();
            else
                start_element_no_attribs("direction", m_pObj);

            //content
            add_direction_type();
            add_offset();
            add_editorial_voice();
            add_staff_number();
            add_sound();

            end_element();
        }

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    bool is_empty_direction()
    {
        ImoAttachments* pAuxObjs = m_pObj->get_attachments();
        ImoSoundChange* pSound = static_cast<ImoSoundChange*>(
                                    m_pObj->get_child_of_type(k_imo_sound_change) );
        return ((pAuxObjs == nullptr || (pAuxObjs && pAuxObjs->get_num_items() == 0))
                && pSound == nullptr
                && m_pObj->get_num_relations() == 0
                && m_pObj->get_noterest_for_transferred_dynamics() == nullptr);
    }

    //-----------------------------------------------------------------------------------
    void add_direction_type()
    {
        add_pedal_mark();
        add_transferred_dynamics();
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
                ; //ignore. It has been already exported

            else
            {
                stringstream msg;
                msg << "Direction not supported in MusicXL exporter. AuxObj="
                    << pAO->get_name();
                LOMSE_LOG_ERROR(msg.str());
                m_source << "<!-- " << msg.str() << " -->";
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_coda_segno(ImoSymbolRepetitionMark* pImo)
    {
//    %print-style-align;
//    add_attribute_optional_unique_id(pImo);
//    %smufl;
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
        start_element_no_attribs("direction-type");

        start_element("words", pImo);
        add_attibutes_for_text_formatting(pImo);
        close_start_tag();
        m_source << pImo->get_text();
        end_element(k_in_same_line);

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_dynamics(ImoDynamicsMark* pImo)
    {
        start_element_no_attribs("direction-type");

//<!ATTLIST dynamics
//    %print-style-align;
//    %placement;
//    %text-decoration;
//    %enclosure;
//    add_attribute_optional_unique_id(pImo);
//>
        start_element_no_attribs("dynamics", pImo);
        m_source << "<" << pImo->get_mark_type() << "/>";
        end_element(k_in_same_line);

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_metronome(ImoMetronomeMark* pImo)
    {
//<!ATTLIST metronome
//    %print-style-align;
//    %print-object;
//    %justify;
//    parentheses %yes-no; #IMPLIED
//    add_attribute_optional_unique_id(pImo);
//>
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
    void add_pedal_mark()
    {
        //TODO
        //<!ATTLIST pedal
        //    type (start | stop | sostenuto | change |
        //          continue | discontinue | resume) #REQUIRED
        //    number %number-level; #IMPLIED
        //    line %yes-no; #IMPLIED
        //    sign %yes-no; #IMPLIED
        //    abbreviated %yes-no; #IMPLIED
        //    %print-style-align;
        //    add_attribute_optional_unique_id(pMark);
        //>

        //look for a ImoPedalMark
        ImoPedalMark* pMark = nullptr;
        ImoAttachments* pAuxObjs = m_pObj->get_attachments();
        if (pAuxObjs)
            pMark = static_cast<ImoPedalMark*>(
                        pAuxObjs->find_item_of_type(k_imo_pedal_mark) );

        //look for a ImoPedalLine
        ImoPedalLine* pLine = nullptr;
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            pLine = static_cast<ImoPedalLine*>(
                        pRelObjs->find_item_of_type(k_imo_pedal_line) );
        }

        if (pMark || pLine)
        {
            start_element_no_attribs("direction-type");

            start_element("pedal");

            //type attributte
            string type;
            if (pMark)
            {
                switch (pMark->get_type())
                {
                    case k_pedal_mark_start:            type = "start";     break;
                    case k_pedal_mark_sostenuto_start:  type = "sostenuto"; break;
                    case k_pedal_mark_stop:             type = "stop";      break;
                    default:
                    {
                        LOMSE_LOG_ERROR("Invalid pedal mark type: k_pedal_mark_unknown");
                        type = "start";
                    }
                }
            }
            else
            {
                if (pLine->get_start_object() == m_pObj)
                    type = (pLine->get_draw_start_corner() ? "start" : "resume");
                else if (pLine->get_end_object() == m_pObj)
                    type = (pLine->get_draw_end_corner() ? "stop" : "discontinue");
                else
                    type = "change";
            }
            add_attribute("type", type);

            //line and sign attributes
            add_attribute("line", pLine ? "yes" : "no");
            if (pMark && pLine)
                add_attribute("sign", "yes");

            end_element(false, false);  //pedal
            end_element();  //direction-type
        }
    }

    //-----------------------------------------------------------------------------------
    void add_transferred_dynamics()
    {
        ImoNoteRest* pNR = m_pObj->get_noterest_for_transferred_dynamics();
        if (pNR)
        {
            ImoAttachments* pAuxObjs = pNR->get_attachments();
            if (pAuxObjs)
            {
                ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(
                                            pAuxObjs->find_item_of_type(k_imo_dynamics_mark) );

                if (pImo && pImo->is_transferred_from_direction())
                    add_dynamics(pImo);
            }
        }
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
                    //ignore. It has been already exported
                }
                else
                {
                    stringstream msg;
                    msg << "Direction not supported in MusicXL exporter. RelObj="
                        << pRO->get_name();
                    LOMSE_LOG_ERROR(msg.str());
                    m_source << "<!-- " << msg.str() << " -->";
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

        //TODO
        //    number %number-level; #IMPLIED
        //    spread %tenths; #IMPLIED
        //    niente %yes-no; #IMPLIED
        //    %line-type;
        //    %dashed-formatting;
        add_attributes_for_position(pImo);
        //    %color;
        add_attribute_optional_unique_id(pImo);

        end_element(false, false);    //wedge

        end_element();  //direction-type
    }

    //-----------------------------------------------------------------------------------
    void add_offset()
    {
        //TODO   Not yet imported
        // offset?
    }

    //-----------------------------------------------------------------------------------
    void add_editorial_voice()
    {
        //TODO   Not yet imported
        // %editorial-voice; = (footnote?, level?, voice?)
    }

    //-----------------------------------------------------------------------------------
    void add_staff_number()
    {
        ImoInstrument* pInstr = m_pExporter->get_current_instrument();
        //AWARE in some unit test instrument does not exist
        if (pInstr && pInstr->get_num_staves() > 1)
            create_element("staff", m_pObj->get_staff() + 1);
    }

    //-----------------------------------------------------------------------------------
    void add_sound()
    {
        ImoSoundChange* pSound = static_cast<ImoSoundChange*>(
                                    m_pObj->get_child_of_type(k_imo_sound_change) );
        if (pSound)
            add_source_for(pSound);
    }

};


//---------------------------------------------------------------------------------------
class ErrorMxlGenerator : public MxlGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source() override
    {
        stringstream msg;
        msg << "Error: no MxlExporter for Imo=" << m_pImo->get_name()
            << ", Imo type=" << m_pImo->get_obj_type()
            << ", id=" << m_pImo->get_id();
        LOMSE_LOG_ERROR(msg.str());

        create_element("TODO", msg.str());
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class FermataMxlGenerator : public MxlGenerator
{
protected:
    ImoFermata* m_pImo;

public:
    FermataMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo(static_cast<ImoFermata*>(pImo))
    {
    }

    string generate_source() override
    {
        start_element("fermata");

        //attrib: type
        if (m_pImo->get_placement() == k_placement_below)
            add_attribute("type", "inverted");
        else if (m_pImo->get_placement() == k_placement_above)
            add_attribute("type", "upright");

        if (m_pImo->get_symbol() == ImoFermata::k_normal)
            end_element(false, false);
        else
        {
            close_start_tag();

            switch (m_pImo->get_symbol())
            {
                case ImoFermata::k_short:       m_source << "angled";           break;
                case  ImoFermata::k_long:       m_source << "square";           break;
                case ImoFermata::k_very_short:  m_source << "double-angled";    break;
                case ImoFermata::k_very_long:   m_source << "double-square";    break;
                case ImoFermata::k_henze_long:  m_source << "double-dot";       break;
                case ImoFermata::k_henze_short: m_source << "half-curve";       break;
                case ImoFermata::k_curlew:      m_source << "curlew";           break;
                default:
                {
                    stringstream msg;
                    msg << "Fermata symbol " << m_pImo->get_symbol()
                        << " is not supported. Program maintenance error?";
                    LOMSE_LOG_ERROR(msg.str());
                    m_source << "normal";
                }
            }
            end_element(k_in_same_line);
        }
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class InstrumentMxlGenerator : public MxlGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
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
    bool m_fExportNumber;

public:
    KeySignatureMxlGenerator(ImoObj* pImo, MxlExporter* pExporter, bool fExportNumber=false)
        : MxlGenerator(pExporter)
        , m_pObj(static_cast<ImoKeySignature*>(pImo))
        , m_fExportNumber(fExportNumber)
    {
    }

    KeySignatureMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pObj(static_cast<ImoKeySignature*>(pImo))
        , m_fExportNumber(false)
    {
    }

    string generate_source() override
    {
        start_element_if_not_started("attributes");

        start_element("key", m_pObj);
        if (m_fExportNumber)
            add_staff_number();
        add_attributes_for_print_style(m_pObj);
        add_attribute_print_object(m_pObj);
        add_attribute_optional_unique_id(m_pObj);
        close_start_tag();

        add_content();

        end_element();
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

    void add_content()
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
            static const string sStepName[7] = { "C",  "D", "E", "F", "G", "A", "B" };
            for (size_t i=0; i < 7; ++i)
            {
                KeyAccidental& acc = m_pObj->get_accidental(i);
                if (acc.step != k_step_undefined && acc.accidental != k_no_accidentals)
                {
                    create_element("key-step", sStepName[acc.step]);
                    create_element("key-alter", acc.alter);
                    create_element("key-accidental",
                                   MxlExporter::accidentals_to_mxl_name(acc.accidental));
                }
            }
            for (size_t i=0; i < 7; ++i)
            {
                if (m_pObj->get_octave(i) != -1)
                {
                    start_element("key-octave");
                    add_attribute("number", int(i+1));
                    close_start_tag();
                    m_source << m_pObj->get_octave(i);
                    end_element(k_in_same_line);
                }
            }
        }
    }

};


//---------------------------------------------------------------------------------------
class LenmusdocMxlGenerator : public MxlGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source() override
    {
        //export first score
        ImoContent* pContent = m_pObj->get_content();
        ImoObj::children_iterator it;
        for (it= pContent->begin(); it != pContent->end(); ++it)
        {
            if ((*it)->is_score())
            {
                ImoScore* pScore = static_cast<ImoScore*>(*it);
                add_source_for( pScore );
                return m_source.str();
            }
        }
        return "";
    }

protected:
};


//---------------------------------------------------------------------------------------
class LyricMxlGenerator : public MxlGenerator
{
protected:
    ImoLyric* m_pObj;
    ImoNoteRest* m_pNR;

public:
    LyricMxlGenerator(ImoObj* pImo, ImoNoteRest* pNR, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pNR(pNR)
    {
        m_pObj = static_cast<ImoLyric*>(pImo);
    }

    string generate_source() override
    {
        start_element("lyric", m_pObj);
        add_attribute("number", m_pObj->get_number());
        if (m_pObj->get_placement() != k_placement_default)
        {
            add_attribute("placement",
                m_pObj->get_placement() == k_placement_above ? "above" : "below");
        }
        close_start_tag();
        //TODO
        //<!ATTLIST lyric
        //    number NMTOKEN #IMPLIED
        //    name CDATA #IMPLIED
        //    %justify;
        //    %position;
        //    %placement;
        //    %color;
        //    %print-object;
        //    %time-only;
        add_attribute_optional_unique_id(m_pObj);

        int numSyllables = m_pObj->get_num_text_items();
        for (int i=0; i < numSyllables; ++i)
        {
            ImoLyricsTextInfo* pText = m_pObj->get_text_item(i);
            add_syllabic(pText);
            add_text(pText);
            add_elision(pText);
            add_extend(pText);
        }

        end_element();  //lyric
        return m_source.str();
    }

protected:
    //-----------------------------------------------------------------------------------
    void add_syllabic(ImoLyricsTextInfo* pText)
    {
        switch (pText->get_syllable_type())
        {
            case ImoLyricsTextInfo::k_single:
                create_element("syllabic", "single");   break;
            case ImoLyricsTextInfo::k_begin:
                create_element("syllabic", "begin");    break;
            case ImoLyricsTextInfo::k_end:
                create_element("syllabic", "end");      break;
            case ImoLyricsTextInfo::k_middle:
                create_element("syllabic", "middle");   break;
        }
    }

    //-----------------------------------------------------------------------------------
    void add_text(ImoLyricsTextInfo* pText)
    {
        create_element("text", pText->get_syllable_text());
        //TODO
        //<!ATTLIST text
        //    %font;
        //    %color;
        //    %text-decoration;
        //    %text-rotation;
        //    %letter-spacing;
        //    xml:lang CDATA #IMPLIED
        //    %text-direction;
        //>
    }

    //-----------------------------------------------------------------------------------
    void add_elision(ImoLyricsTextInfo* pText)
    {
        if (pText->has_elision())
            create_element("elision", "‿"); //pText->get_elision_text());
        //TODO
        //<!ATTLIST elision
        //    %font;
        //    %color;
        //    %smufl;
        //>
    }

    //-----------------------------------------------------------------------------------
    void add_extend(ImoLyricsTextInfo* UNUSED(pText))
    {
        if (m_pObj->has_melisma())
        {
            start_element("extend");
            //TODO: LDP model was based on MusicXML v3.0 and thus, extend is present only
            // in start note. Before MusicXML v3.0 the extend attribute "type" was not
            // available, and an <extend> element was always treated as the start of the
            // melisma line. And the end has to be deduced. But there is no reliable way
            // to determine where it is supposed to stop.
            //It is necessary to modify the IM for also including the end of melisma lines.

            //TODO
            //<!ATTLIST extend
            //    type %start-stop-continue; #IMPLIED
            //    %position;
            //    %color;
            //>
            //add_attribute("type", "start");
            end_element(false, false);
        }
    }

};


//---------------------------------------------------------------------------------------
class MidiInfoMxlGenerator : public MxlGenerator
{
protected:
    ImoMidiInfo* m_pImo;

public:
    MidiInfoMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoMidiInfo*>(pImo))
    {
    }

    string generate_source() override
    {
        if (!m_pImo->only_contains_default_values())
        {
            start_element("midi-instrument");
            add_attribute("id", m_pImo->get_score_instr_id());
            close_start_tag();

            // midi-channel
            if (m_pImo->is_channel_modified())
                create_element("midi-channel", m_pImo->get_midi_channel() + 1);

            // midi-name
            if (m_pImo->is_name_modified())
                create_element("midi-name", m_pImo->get_midi_name());

            // midi-bank
            if (m_pImo->is_bank_modified())
                create_element("midi-bank", m_pImo->get_midi_bank() + 1);

            // midi-program
            if (m_pImo->is_program_modified())
                create_element("midi-program", m_pImo->get_midi_program() + 1);

            // midi-unpitched
            if (m_pImo->is_unpitched_modified())
                create_element("midi-unpitched", m_pImo->get_midi_unpitched() + 1);

            // volume
            if (m_pImo->is_volume_modified())
                create_element("volume", m_pImo->get_midi_volume() * 100.0f);

            // pan
            if (m_pImo->is_pan_modified())
                create_element("pan", m_pImo->get_midi_pan());

            // elevation
            if (m_pImo->is_elevation_modified())
                create_element("elevation", m_pImo->get_midi_elevation());

            end_element();  //midi-instrument
        }
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class TimeSignatureMxlGenerator : public MxlGenerator
{
protected:
    ImoTimeSignature* m_pObj;
    bool m_fExportNumber;

public:
    TimeSignatureMxlGenerator(ImoObj* pImo, MxlExporter* pExporter, bool fExportNumber=false)
        : MxlGenerator(pExporter)
        , m_pObj(static_cast<ImoTimeSignature*>(pImo))
        , m_fExportNumber(fExportNumber)
    {
    }

    TimeSignatureMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pObj(static_cast<ImoTimeSignature*>(pImo))
        , m_fExportNumber(false)
    {
    }

    string generate_source() override
    {
        start_element_if_not_started("attributes");

        start_element("time", m_pObj);
        if (m_fExportNumber)
            add_staff_number();
        add_symbol();
        //TODO  attrb: %time-separator;
        add_attributes_for_print_style_align(m_pObj);
        add_attribute_print_object(m_pObj);
        add_attribute_optional_unique_id(m_pObj);
        close_start_tag();

        add_beats_type();
        //TODO
        //interchangeable
        //senza-misura

        end_element();
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


//---------------------------------------------------------------------------------------
class MusicDataMxlGenerator : public MxlGenerator
{
protected:
    ImoMusicData* m_pObj;
    ImoScore* m_pScore;

public:
    MusicDataMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
        m_pScore = m_pExporter->get_current_score();
    }

    string generate_source() override
    {
        add_measures();
        return m_source.str();
    }

protected:

    int m_curMeasure;
    int m_iInstr;
    list<int> m_voices;
    ColStaffObjs* m_pColStaffObjs;
    ColStaffObjsIterator m_itStartOfMeasure;    //first object or current position
    ColStaffObjsIterator m_itEndOfMeasure;      //barline for instr,measure or end pos.
    ColStaffObjsIterator m_itStartOfNextMeasure;    //could be before m_itEndOfMeasure as
                                                    //ColStaffObjs is not ordered by measure
    bool m_fDivisionsAdded = false;

#define LOMSE_USE_LINES   0

    //-----------------------------------------------------------------------------------
    void add_measures()
    {
        ImoInstrument* pInstr = m_pExporter->get_current_instrument();
        m_iInstr = m_pScore->get_instr_number_for(pInstr);
        m_pColStaffObjs = m_pScore->get_staffobjs_table();

        if (m_pColStaffObjs->num_entries() < 1)
            return;

        m_itStartOfNextMeasure = m_pColStaffObjs->begin();
        advance_to_right_instrument();

        ColStaffObjsIterator it = m_itStartOfMeasure;
        while (it != m_pColStaffObjs->end())
        {
///*DBG*/            cout << endl << "measure start: " << (*it)->imo_object()->get_id() <<endl;
            add_measure();

            find_start_of_next_measure();
            it = m_itStartOfMeasure;
///*DBG*/     cout << "measure ended: ";
///*DBG*/     if (m_itStartOfMeasure != m_pColStaffObjs->end())
///*DBG*/         cout << "next measure starts at " << (*m_itStartOfMeasure)->imo_object()->get_id() <<endl;
///*DBG*/     else
///*DBG*/         cout << "end of score" << endl;
        }
    }

    //-----------------------------------------------------------------------------------
    void add_measure()
    {
        //measure starts at m_itStartOfMeasure

///*DBG*/ cout << "    add_measure(). m_curMeasure = " << m_curMeasure
///*DBG*/      << ", m_iInstr = " << m_iInstr << endl;
        count_voices_in_measure();

        start_measure_element();

        add_attributes_at_start_of_measure();
        add_source_for_this_measure();
        add_source_for_barline();

        end_element();  //measure
    }

    //-----------------------------------------------------------------------------------
    void count_voices_in_measure()
    {
        //measure starts at m_itStartOfMeasure. Identify existing lines in this measure
        //and locate end of measure (its barline). While traversing, identify first
        //object for next measure, if exist

        m_voices.clear();
        ColStaffObjsIterator it = m_itStartOfMeasure;
        if (it != m_pColStaffObjs->end())
        {
///*DBG*/     cout << "    count_voices_in_measure(). start: " << (*it)->imo_object()->get_id() <<endl;
            int voice = 0;
            while (it != m_pColStaffObjs->end())
            {
                if ((*it)->measure() == m_curMeasure)
                {
                    if (!(*it)->imo_object()->is_barline())
                    {
                        if ((*it)->num_instrument() == m_iInstr)
                        {
                        #if (LOMSE_USE_LINES == 1)
                            m_voices.push_back( (*it)->line() );
                        #else
                            ImoStaffObj* pSO = (*it)->imo_object();
                            if (pSO->is_note_rest())
                                voice = static_cast<ImoNoteRest*>(pSO)->get_voice();
                            else if (pSO->is_direction())
                                voice = static_cast<ImoDirection*>(pSO)->get_voice();

                            m_voices.push_back(voice);
                       #endif
///*DBG2*/                           cout << "object " << pSO->get_name() << " assigned to voice " << voice << endl;
         }
                    }
                    else /* barline for this measure */
                    {
                        if ((*it)->num_instrument() == m_iInstr)
                            break;
                    }
                }
                else if (m_itStartOfNextMeasure == m_pColStaffObjs->end()
                         && (*it)->measure() > m_curMeasure)
                {
///*DBG*/             cout << "    count_voices_in_measure(). StartOfNextMeasure is empty. Assigning: " << (*it)->imo_object()->get_id() <<endl;
                    m_itStartOfNextMeasure = it;
                }

                ++it;
            }
            m_voices.sort();
            m_voices.unique();
///*DBG2*/           cout << endl << "voices: ";
///*DBG2*/           for (auto i : m_voices)
///*DBG2*/           {
///*DBG2*/               cout << i << ", ";
///*DBG2*/           }
        }

        m_itEndOfMeasure = it;
///*DBG*/ if (it != m_pColStaffObjs->end())
///*DBG*/     cout << "    count_voices_in_measure(). end for this measure: " << (*it)->imo_object()->get_id() <<endl;
///*DBG*/ else
///*DBG*/     cout << "    count_voices_in_measure(). end for this measure is end of StaffObjsCol." << endl;

        //if start of next measure not set, use end of this measure as next start point
        if (m_itStartOfNextMeasure == m_pColStaffObjs->end())
        {
///*DBG*/     cout << "    count_voices_in_measure(). StartOfNextMeasure is empty. Assigning end of this one." <<endl;
            m_itStartOfNextMeasure = m_itEndOfMeasure;
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_this_measure()
    {
        bool fFirstLine = true;
        for (auto i : m_voices)
        {
            if (!fFirstLine)
                add_backup();
            add_source_for_voice(i);
            fFirstLine = false;
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_barline()
    {
        ColStaffObjsIterator it = m_itEndOfMeasure;
        if (it != m_pColStaffObjs->end() && (*it)->imo_object()->is_barline()
            && (*it)->num_instrument() == m_iInstr && (*it)->measure() == m_curMeasure)
        {
            add_source_for((*it)->imo_object());
        }
    }

    //-----------------------------------------------------------------------------------
    void find_start_of_next_measure()
    {
        //measure is finished and m_itStartOfNextMeasure points to end barline or to
        //a previous object for next measure. If points to barline, advance to first
        //object for next measure

        ColStaffObjsIterator it = m_itStartOfNextMeasure;
///*DBG*/ if (it != m_pColStaffObjs->end())
///*DBG*/            cout << "    find_start_of_next_measure(). StartOfNextMeasure: " << (*it)->imo_object()->get_id() <<endl;
        while (it != m_pColStaffObjs->end() && (*it)->imo_object()->is_barline())
        {
            ++it;
///*DBG*/            if (it != m_pColStaffObjs->end())
///*DBG*/         cout << "    find_start_of_next_measure(). incrementing to " << (*it)->imo_object()->get_id() <<endl;
///*DBG*/     else
///*DBG*/         cout << "    find_start_of_next_measure(). incrementing to end of collection." <<endl;
        }
        m_itStartOfNextMeasure = it;
///*DBG*/        if (it != m_pColStaffObjs->end())
///*DBG*/     cout << "    find_start_of_next_measure(). StartOfNextMeasure: " << (*it)->imo_object()->get_id() <<endl;
///*DBG*/ else
///*DBG*/     cout << "    find_start_of_next_measure(). StartOfNextMeasure is end of collection." <<endl;

        advance_to_right_instrument();
    }

    //-----------------------------------------------------------------------------------
    void advance_to_right_instrument()
    {
        //sets m_itStartOfMeasure and avances it if not in right instrument. Also
        //sets m_curMeasure

        m_itStartOfMeasure = m_itStartOfNextMeasure;
        m_itStartOfNextMeasure = m_pColStaffObjs->end();

        ColStaffObjsIterator it = m_itStartOfMeasure;
///*DBG*/  if (it != m_pColStaffObjs->end())
///*DBG*/     cout << "    advance_to_right_instrument(). StartOfMeasure: " << (*it)->imo_object()->get_id() <<endl;
        if (it != m_pColStaffObjs->end())
        {
            while (it != m_pColStaffObjs->end() && (*it)->num_instrument() != m_iInstr)
                ++it;
            m_itStartOfMeasure = it;

            if (it != m_pColStaffObjs->end())
                m_curMeasure = (*it)->measure();
        }
///*DBG*/  if (it != m_pColStaffObjs->end())
///*DBG*/     cout << "                                   StartOfMeasure: " << (*it)->imo_object()->get_id() <<endl;
    }

    //-----------------------------------------------------------------------------------
    void add_attributes_at_start_of_measure()
    {
///*DBG*/ if (m_itStartOfMeasure != m_pColStaffObjs->end())
///*DBG*/     cout << "    add_attributes_at_start_of_measure(). m_itStartOfMeasure: " << (*m_itStartOfMeasure)->imo_object()->get_id() <<endl;

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
    void start_measure_element()
    {
        add_separator_line();
        start_element("measure");
        start_attrib("number");
        m_source << m_curMeasure+1;
        end_attrib();
        close_start_tag();
        m_pExporter->set_current_timepos(0);
        add_left_barline();
    }

    //-----------------------------------------------------------------------------------
    void add_left_barline()
    {
        BarlineMxlGenerator exporter(nullptr, nullptr, m_pExporter);
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
        ImoStaffObj* prevKey = nullptr;
        ImoStaffObj* prevTime = nullptr;
        while (it != m_itEndOfMeasure)
        {
            if ((*it)->num_instrument() == m_iInstr && (*it)->measure() == m_curMeasure)
            {
///*DBG*/     cout << "                   Collecting attribute " << (*it)->imo_object()->get_id() <<endl;
                ImoStaffObj* pImo = (*it)->imo_object();
                if (pImo->is_clef())
                    clefs.push_back(pImo);
                else if (pImo->is_key_signature())
                {
                    if (pImo != prevKey)    //filter out copies from 1st staff
                        keys.push_back(pImo);
                    prevKey = pImo;
                }
                else if (pImo->is_time_signature())    //filter out copies from 1st staff
                {
                    if (pImo != prevTime)
                        times.push_back(pImo);
                    prevTime = pImo;
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
            {
                KeySignatureMxlGenerator exporter(obj, m_pExporter, keys.size() != 1);
                m_source << exporter.generate_source();
            }

            //time*
            for (auto obj : times)
            {
                TimeSignatureMxlGenerator exporter(obj, m_pExporter, times.size() != 1);
                m_source << exporter.generate_source();
            }

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

        add_staff_details();
        //TODO: transpose*
        //TODO: directive*
        //TODO: measure-style*
    }

    //-----------------------------------------------------------------------------------
    void add_staff_details()
    {
        ImoInstrument* pInstr = m_pObj->get_instrument();
        if (pInstr == nullptr)
            return;

        for (int i=0; i < pInstr->get_num_staves(); ++i)
        {
            ImoStaffInfo* pInfo = pInstr->get_staff(i);
            if (pInfo && !pInfo->has_default_values())
            {
                start_element_if_not_started("attributes");
                start_element("staff-details");
                if (pInstr->get_num_staves() > 1)
                    add_attribute("number", i+1);
                //TODO
                //attrib: show-frets     (numbers | letters)  #IMPLIED
                //attrib: %print-object;
                //attrib: %print-spacing;
                close_start_tag();

                //staff-type
                switch(pInfo->get_staff_type())
                {
                    case ImoStaffInfo::k_staff_alternate:
                        create_element("staff-type", "alternate");  break;
                    case ImoStaffInfo::k_staff_cue:
                        create_element("staff-type", "cue");        break;
                    case ImoStaffInfo::k_staff_editorial:
                        create_element("staff-type", "editorial");  break;
                    case ImoStaffInfo::k_staff_ossia:
                        create_element("staff-type", "ossia");      break;
                    default:
                        ;
                }

                //staff-lines
                if (pInfo->get_num_lines() != 5)
                    create_element("staff-lines", pInfo->get_num_lines());

                //TODO: Not yet supported/imported. ImoStaffInfo not yet support this
                //line-detail*
                //<!ELEMENT line-detail EMPTY>
                //<!ATTLIST line-detail
                //    line    CDATA       #REQUIRED
                //    width   %tenths;    #IMPLIED
                //    %color;
                //    %line-type;
                //    %print-object;
                //>

                //TODO: Not yet supported/imported. For tablature
                //staff-tuning*
                    //<!ELEMENT staff-tuning
                    //	(tuning-step, tuning-alter?, tuning-octave)>
                    //<!ATTLIST staff-tuning
                    //    line CDATA #REQUIRED
                    //>

                //TODO: Not yet supported/imported. For tablature
                //capo?
                    //<!ELEMENT capo (#PCDATA)>

                //staff-size
                if (pInfo->is_staff_size_modified())
                {
                    start_element("staff-size");
                    add_attribute("scaling", 100);
                    close_start_tag();
                    m_source << (100.0 * pInfo->get_applied_spacing_factor());
                    end_element(k_in_same_line);
                }
                end_element();  //staff-details
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_voice(int voice)
    {
///*DBG2*/      cout << endl << "processing voice " << line << endl;
///*DBG*/ if (m_itStartOfMeasure != m_pColStaffObjs->end())
///*DBG*/     cout << "    add_source_for_voice(). first no attribe obj: " << (*m_itStartOfMeasure)->imo_object()->get_id() <<endl;
    #if (LOMSE_USE_LINES == 1)
        ColStaffObjsIterator it;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            ColStaffObjsEntry* pEntry = *it;
            if (pEntry->num_instrument() == m_iInstr && pEntry->measure() == m_curMeasure)
            {
                if (pEntry->line() == voice)
                {
                    if (pEntry->measure() == m_curMeasure)
                        add_source_for( pEntry->imo_object() );
                }
            }
        }
    #else
        ColStaffObjsIterator it;
        int curVoice = 0;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            ColStaffObjsEntry* pEntry = *it;
            if (pEntry->num_instrument() == m_iInstr && pEntry->measure() == m_curMeasure)
            {
                ImoStaffObj* pSO = (*it)->imo_object();
                if (pSO->is_note_rest())
                    curVoice = static_cast<ImoNoteRest*>(pSO)->get_voice();
                else if (pSO->is_direction())
                    curVoice = static_cast<ImoDirection*>(pSO)->get_voice();

///*DBG2*/               cout << "    processing " << pSO->get_name() << " in voice " << curVoice << endl;
                if (voice == curVoice)
                    add_source_for(pSO);
            }
        }
    #endif
    }

    //-----------------------------------------------------------------------------------
    void add_backup()
    {
        if (m_pExporter->get_current_timepos() > 0)
        {
            start_element_no_attribs("backup");
            create_element("duration", m_pExporter->get_current_timepos());
            end_element();  //backup
        }
    }

};


//---------------------------------------------------------------------------------------
class OctaveShiftMxlGenerator : public MxlGenerator
{
protected:
    ImoOctaveShift* m_pObj;
    std::string m_type;

public:
    OctaveShiftMxlGenerator(ImoObj* pImo, const std::string& type, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_type(type)
    {
        m_pObj = static_cast<ImoOctaveShift*>(pImo);
    }

    string generate_source() override
    {
        start_element_no_attribs("direction", m_pObj);
        add_octave_shift();
        end_element();

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_octave_shift()
    {
        start_element_no_attribs("direction-type");

        start_element("octave-shift", m_pObj);

        int size = m_pObj->get_shift_steps();
        if (m_type == "start")
            add_attribute("type", size > 0 ? "up" : "down");
        else
            add_attribute("type", m_type);
        add_attribute("size", abs(size) + 1);
        add_attribute("number", m_pObj->get_octave_shift_number());
        //TODO
        //    %dashed-formatting;
        add_attributes_for_print_style(m_pObj);
        //add_attribute_optional_unique_id(m_pObj);
        end_element(false, false);    //octave-shift

        end_element();  //direction-type
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
    NoteRestMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pNR = static_cast<ImoNoteRest*>(pImo);
        if (pImo->is_rest())
            m_pRest = static_cast<ImoRest*>(pImo);
        else
            m_pNote = static_cast<ImoNote*>(pImo);
    }

    string generate_source() override
    {
        end_element_if_started("attributes");

        //AWARE: octave-shifts are modelled as a RelObj relating all notes. Therefore,
        //first note/rest will contain the start of an octave shift and must be exported
        //as an independent direction before exporting the note/rest
        add_octave_shift_start();

        //Now export the note/rest

        if (m_pRest && m_pRest->is_gap())
        {
            add_forward();
            return m_source.str();
        }


        start_element("note", m_pNR);
        add_attributes();
        close_start_tag();

        if (m_pNote)
        {
            add_grace();
            add_cue();
            add_chord();
            add_pitch();
        }
        else
        {
            add_rest();
        }

        add_duration();
        add_ties();
        add_voice();
        add_type();
        add_dots();
        add_accidental();
        add_time_modification();
        add_stem();
        //TODO
        //add_notehead();
        //add_notehead_text();
        add_staff(m_pNR);
        add_beam();
        add_notations();
        add_lyrics();
        //TODO
        //add_play();

        end_element();   //note

        //AWARE: octave-shifts are modelled as a RelObj relating all notes. Therefore,
        //last note/rest will contain the end of an octave shift and must be exported
        //as an independent direction after exporting the note/rest
        add_octave_shift_stop();

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_attributes()
    {
        //TODO
        add_attributes_for_print_style(m_pNR);
        add_attributes_for_printout(m_pNR);
        //    print-leger %yes-no; #IMPLIED
        //    dynamics CDATA #IMPLIED
        //    end-dynamics CDATA #IMPLIED
        //    attack CDATA #IMPLIED
        //    release CDATA #IMPLIED
        //    %time-only;
        //    pizzicato %yes-no; #IMPLIED
        add_attribute_optional_unique_id(m_pNR);
    }

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
    void add_grace()
    {
        if (m_pNote->is_grace_note())
        {
            ImoGraceNote* pGrace = static_cast<ImoGraceNote*>(m_pNote);

            start_element("grace");

            ImoRelations* pRelObjs = pGrace->get_relations();
            ImoGraceRelObj* pRO =
                static_cast<ImoGraceRelObj*>( pRelObjs->find_item_of_type(k_imo_grace_relobj));

            if (!m_pNote->is_in_chord() || m_pNote->is_start_of_chord())
            {
                if (pRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_previous)
                    add_attribute("steal-time-previous", pRO->get_percentage() * 100.0f);
                else if (pRO->get_grace_type() == ImoGraceRelObj::k_grace_steal_following)
                    add_attribute("steal-time-following", pRO->get_percentage() * 100.0f);
            }

            //TODO   attrib: make-time

            if (pRO->has_slash())
                add_attribute("slash", "yes");

            end_element(false, false);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_cue()
    {
        if (m_pNote->is_cue_note())
            empty_element("cue");
    }

    //-----------------------------------------------------------------------------------
    void add_rest()
    {
        static const string sStep[7] = { "C",  "D", "E", "F", "G", "A", "B" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };
        start_element("rest");
        if (m_pRest->is_full_measure())
            add_attribute("measure", "yes");

        if (m_pRest->get_step() != k_step_undefined)
        {
            close_start_tag();
            create_element("display-step", sStep[m_pRest->get_step()] );
            create_element("display-octave", sOctave[m_pRest->get_octave()] );
            end_element();
        }
        else
            end_element(false, false);
    }

    //-----------------------------------------------------------------------------------
    void add_pitch()
    {
        static const string sNoteName[7] = { "C",  "D", "E", "F", "G", "A", "B" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pNR->is_unpitched())
        {
            start_element_no_attribs("unpitched");
            create_element("display-step", sNoteName[m_pNR->get_step()] );
            create_element("display-octave", sOctave[m_pNR->get_octave()] );
            end_element();  //unpitched
            return;
        }

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
        if (!m_pNR->is_grace_note())
            create_element("duration", dur);

        if (m_pRest || !m_pNote->is_in_chord() || m_pNote->is_start_of_chord())
            m_pExporter->increment_current_timepos(dur);
    }

    //-----------------------------------------------------------------------------------
    void add_ties()
    {
        //TODO:   atrib: %time-only;
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
            end_element(k_in_same_line);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_dots()
    {
        for (int dots = m_pNR->get_dots(); dots > 0; --dots)
        {
            empty_element("dot");
            //TODO
            //<!ATTLIST dot
            //    %print-style;
            //    %placement;
            //>
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
                    //TODO
                    //<!ATTLIST accidental
                    //    cautionary %yes-no; #IMPLIED
                    //    editorial %yes-no; #IMPLIED
                    //    %level-display;
                    //    %print-style;
                    //    %smufl;
                    //>
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
                //TODO
                //<!ATTLIST stem
                //    %position;
                //    %color;
                //>
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
            BeamMxlGenerator gen(m_pNote, nullptr, m_pExporter);
            m_source << gen.generate_source();
        }
    }

    //-----------------------------------------------------------------------------------
    void add_notations()
    {
        //TODO
        //<!ATTLIST tied
        //    type %tied-type; #REQUIRED
        //    number %number-level; #IMPLIED
        //    %line-type;
        //    %dashed-formatting;
        //    %position;
        //    %placement;
        //    %orientation;
        //    %bezier;
        //    %color;
        //    add_attribute_optional_unique_id(m_pObj);
        //>
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
    void add_lyrics()
    {
        ImoAttachments* pAuxObjs = m_pNR->get_attachments();
        if (pAuxObjs == nullptr)
            return;

        ImoObj::children_iterator it;
        for (it=pAuxObjs->begin(); it != pAuxObjs->end(); ++it)
        {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>(*it);
            if (pAO->is_lyric())
            {
                LyricMxlGenerator exporter(pAO, m_pNR, m_pExporter);
                m_source << exporter.generate_source();
            }
        }
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
                if (pRO->is_tuplet() || pRO->is_slur() || pRO->is_arpeggio())
                {
                    add_source_for(pRO, m_pNote);
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
                FermataMxlGenerator exporter(pAO, nullptr, m_pExporter);
                m_source << exporter.generate_source();
            }
            else if (pAO->is_articulation() )
                articulations.push_back(pAO);
            else if (pAO->is_dynamics_mark() )
            {
                //AWARE: Dynamics can be either a notation (when attached to ImoNoteRest)
                //or adirection (when attached to ImoDirection)
                //To simplify layout, MusicXML importer changes dynamic mark irections
                //to note/rest attachment. These moved dynamic-marks must not be exported
                //here as they will be exported ithe preceeding ImoDirection
                ImoDynamicsMark* pImo = static_cast<ImoDynamicsMark*>(pAO);
                if (!pImo->is_transferred_from_direction())
                    dynamics.push_back(pAO);
            }
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
                add_source_for(pAO);
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
//            case k_articulation_strong_accent:      type = "strong-accent";   break;
//            case k_articulation_detached_legato:    type = "detached-legato"; break;
//            case k_articulation_legato_duro:     type = "legato-duro
            case k_articulation_marccato:			type = "strong-accent";   break;
//            case k_articulation_marccato_legato,			///< Marccato-legato
//            case k_articulation_marccato_staccato,		///< Marccato-staccato
//            case k_articulation_marccato_staccatissimo,	///< Marccato-staccatissimo
            case k_articulation_mezzo_staccato:     type = "detached-legato"; break;
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
        //TODO: attributes
        //<!ATTLIST accent
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST strong-accent
        //    %print-style;
        //    %placement;
        //    type %up-down; "up"
        //>
        //<!ATTLIST staccato
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST tenuto
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST detached-legato
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST staccatissimo
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST spiccato
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST scoop
        //    %line-shape;
        //    %line-type;
        //    %line-length;
        //    %dashed-formatting;
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST plop
        //    %line-shape;
        //    %line-type;
        //    %line-length;
        //    %dashed-formatting;
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST doit
        //    %line-shape;
        //    %line-type;
        //    %line-length;
        //    %dashed-formatting;
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST falloff
        //    %line-shape;
        //    %line-type;
        //    %line-length;
        //    %dashed-formatting;
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST breath-mark
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST caesura
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST stress
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST unstress
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST soft-accent
        //    %print-style;
        //    %placement;
        //>
        //<!ATTLIST other-articulation
        //    %print-style;
        //    %placement;
        //    %smufl;
        //>
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

        //TODO
        //<!ATTLIST fret
        //    %font;
        //    %color;
        //>
        start_element_no_attribs("fret");
        m_source << pFS->get_fret();
        end_element(k_in_same_line);

        //TODO
        //<!ATTLIST string
        //    %print-style;
        //    %placement;
        //>
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
            start_element("fingering");

            //attrib: substitution
            if (finger.is_substitution())
                add_attribute("substitution", "yes");

            //attrib: alternate
            if (finger.is_alternative())
                add_attribute("alternate", "yes");

            //attrib: %print_style;
            add_attributes_for_print_style(pSymbol);

            //attrib: %placement;
            //TODO: Not yet imported

            close_start_tag();

            m_source << finger.value;

            end_element(k_in_same_line);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_octave_shift_start()
    {
        if (m_pNR->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pNR->get_relations();
            ImoOctaveShift* pImo = static_cast<ImoOctaveShift*>(
                                        pRelObjs->find_item_of_type(k_imo_octave_shift) );
            if (pImo && pImo->get_start_object() == m_pNR )
            {
                OctaveShiftMxlGenerator exporter(pImo, "start", m_pExporter);
                m_source << exporter.generate_source();
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_octave_shift_stop()
    {
        if (m_pNR->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pNR->get_relations();
            ImoOctaveShift* pImo = static_cast<ImoOctaveShift*>(
                                        pRelObjs->find_item_of_type(k_imo_octave_shift) );

            //AWARE: For octave shift affecting only to one note, start and end notes are
            //the same one.
            if (pImo && (pImo->get_end_object() == m_pNR || pImo->get_start_object() != m_pNR))
            {
                string type = pImo->get_end_object() == m_pNR ? "stop" : "continue";
                OctaveShiftMxlGenerator exporter(pImo, type, m_pExporter);
                m_source << exporter.generate_source();
            }
        }
    }

};

//---------------------------------------------------------------------------------------
class OrnamentMxlGenerator : public MxlGenerator
{
protected:
    ImoOrnament* m_pImo;

public:
    OrnamentMxlGenerator(ImoObj* pImo, ImoObj* UNUSED(pParent), MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoOrnament*>(pImo))
    {
    }

    string generate_source() override
    {
        switch(m_pImo->get_ornament_type())
        {
            case k_ornament_trill_mark:             add_symbol("trill-mark");            break;
            case k_ornament_vertical_turn:          add_symbol("vertical-turn");         break;
            case k_ornament_shake:                  add_symbol("shake");                 break;
            case k_ornament_turn:                   add_symbol("turn");                  break;
            case k_ornament_delayed_turn:           add_symbol("delayed-turn");          break;
            case k_ornament_inverted_turn:          add_symbol("inverted-turn");         break;
            case k_ornament_delayed_inverted_turn:  add_symbol("delayed-inverted-turn"); break;
            case k_ornament_mordent:                add_symbol("mordent");               break;
            case k_ornament_inverted_mordent:       add_symbol("inverted-mordent");      break;
            //TODO: Not yet correctly imported. Cannot be exported
//            case k_ornament_wavy_line:              add_symbol("wavy-line");             break;
            case k_ornament_schleifer:              add_symbol("schleifer");             break;
            case k_ornament_tremolo:                add_tremolo();                       break;
            case k_ornament_other:                  add_symbol("other-ornament");        break;
            default:
                ;
        }

        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_symbol(const string& type)
    {
        if (m_pImo->get_placement() != k_placement_default)
        {
            start_element(type);
            add_attribute("placement", m_pImo->get_placement() == k_placement_above ?
                                         "above" : "below");
            end_element(false, false);
        }
        else
            empty_element(type);
    }

    //-----------------------------------------------------------------------------------
    void add_tremolo()
    {
        start_element("tremolo");

        //TODO   attrib: type %tremolo-type; "single"
        add_attribute("type", "single");

        add_attributes_for_print_style(m_pImo);     //attrib: %print-style;
        //TODO   attrib: %placement;
        //TODO   attrib: %smufl;

        close_start_tag();

        //TODO   content:  tremolo-marks
        //Currently Lomse only imports single tremolos and displays them using
        //a glyph with 3 marks
        m_source << 3;

        end_element();  //tremolo
    }

};

//---------------------------------------------------------------------------------------
class PartListMxlGenerator : public MxlGenerator
{
protected:
    ImoScore* m_pObj;

public:
    PartListMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
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
        start_element("score-part", pInstr);
        add_attribute("id", pInstr->get_instr_id());
        close_start_tag();

        add_identification();
        add_part_name(pInstr->get_name());
        add_part_name_display();
        add_part_abbreviation(pInstr->get_abbrev());
        add_part_abbreviation_display();
        add_groups(pInstr);
        add_instruments_info(pInstr);

        end_element();  //score-part
    }

    //-----------------------------------------------------------------------------------
    void add_identification()
    {
        //TODO
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
        //This is neither imported nor supported in the IM
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
        //This is neither imported nor supported in the IM
    }

    //-----------------------------------------------------------------------------------
    void add_groups(ImoInstrument* pInstr)
    {
        //TODO
        //    group*
    }

    //-----------------------------------------------------------------------------------
    void add_instruments_info(ImoInstrument* pInstr)
    {
        int numSounds = pInstr->get_num_sounds();
        for (int i=0; i < numSounds; ++i)
        {
            ImoSoundInfo* pInfo = pInstr->get_sound_info(i);
            string& instr = pInfo->get_score_instr_name();
            if (!instr.empty())
            {
                add_score_instrument(pInfo);
                add_midi_info(pInfo);
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_score_instrument(ImoSoundInfo* pInfo)
    {
        start_element("score-instrument");
        add_attribute("id", pInfo->get_score_instr_id());
        close_start_tag();

        create_element("instrument-name", pInfo->get_score_instr_name());

        string& abbrev = pInfo->get_score_instr_abbrev();
        if (!abbrev.empty())
            create_element("instrument-abbreviation", abbrev);

        string& sound = pInfo->get_score_instr_sound();
        if (!sound.empty())
            create_element("instrument-sound", sound);

        if (pInfo->get_score_instr_solo())
            empty_element("solo");
        else if (pInfo->get_score_instr_ensemble())
        {
            int num = pInfo->get_score_instr_ensemble_size();
            if (num != 0)
                create_element("ensemble", num);
            else
                empty_element("ensemble");
        }

        string& library = pInfo->get_score_instr_virtual_library();
        string& name = pInfo->get_score_instr_virtual_name();
        if (!library.empty() || !name.empty())
        {
            start_element_no_attribs("virtual-instrument");
            if (!library.empty())
                create_element("virtual-library", library);
            if (!name.empty())
                create_element("virtual-name", name);
            end_element();  //virtual-instrument
        }
        end_element();  //score-instrument
    }

    //-----------------------------------------------------------------------------------
    void add_midi_info(ImoSoundInfo* pInfo)
    {
        ImoMidiInfo* pMidi = pInfo->get_midi_info();

        //midi_device
        string& device = pMidi->get_midi_device_name();
        if (!device.empty())
        {
        //TODO: attrib. port:  Is it necessary to determine if it was present in imported
        //MusicXML file or has been assigned by Lomse?
//        pMidi->get_midi_port() + 1;

            create_element("midi-device", device);
        }

        //midi_instrument
        add_source_for(pMidi);
    }

};


//---------------------------------------------------------------------------------------
class ScoreMxlGenerator : public MxlGenerator
{
protected:
    ImoScore* m_pScore;

public:
    ScoreMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
    {
        m_pScore = static_cast<ImoScore*>(pImo);
        m_pExporter->set_current_score(m_pScore);

        ColStaffObjs* pTable = m_pScore->get_staffobjs_table();
        if (pTable)
            m_pExporter->save_divisions( pTable->get_divisions() );
    }

    string generate_source() override
    {
        add_header();
        start_element("score-partwise", m_pScore);
        add_attribute("version", "4.0");
        close_start_tag();

        add_work();
        add_movement_number();
        add_movement_title();
        add_identification();
        add_defaults();
        add_credits();
        add_part_list();
        add_parts();

        end_element();    //score-partwise
        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_header()
    {
        m_source << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        m_source << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\" "
            << "\"http://www.musicxml.org/dtds/partwise.dtd\">";

        if (!m_pExporter->get_remove_newlines())
        {
            start_comment();
            m_source << "MusicXML file generated by Lomse, version "
                     << m_pExporter->get_version_and_time_string();
            end_comment();
        }
    }

    //-----------------------------------------------------------------------------------
	void add_work()
	{
	    //TODO
	}

    //-----------------------------------------------------------------------------------
	void add_movement_number()
	{
	    //TODO
	}

    //-----------------------------------------------------------------------------------
	void add_movement_title()
	{
	    //TODO
	}

    //-----------------------------------------------------------------------------------
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

    //-----------------------------------------------------------------------------------
	void add_defaults()
	{
	    if (m_pScore->is_global_scaling_modified())
	    {
            start_element_no_attribs("defaults");

            add_scaling();
            add_concert_score();

            //common-layout = (page-layout?, system-layout?, staff-layout*)
            add_page_layout();
            add_system_layout();
            add_staff_layout();

            add_appearance();
            add_music_font();
            add_word_font();
            add_lyric_font();
            add_lyric_language();

            end_element();  //defaults
	    }
	}

    //-----------------------------------------------------------------------------------
	void add_credits()
	{
	    //TODO
	}

    //-----------------------------------------------------------------------------------
    void add_part_list()
    {
        PartListMxlGenerator exporter(m_pScore, nullptr, m_pExporter);
        m_source << exporter.generate_source();
    }

    //-----------------------------------------------------------------------------------
    void add_parts()
    {
        int numInstr = m_pScore->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
        {
            add_source_for(m_pScore->get_instrument(i));
        }
    }

    //-----------------------------------------------------------------------------------
    void add_scaling()
    {
        start_element_no_attribs("scaling");

        float millimeters = m_pScore->tenths_to_logical(40) / 100.0f;
        create_element("millimeters", millimeters);
        create_element("tenths", 40);

        end_element();  //scaling
    }

    //-----------------------------------------------------------------------------------
    void add_concert_score()
    {
        //TODO: Not yet supported in IM. DTD score
        //<!ELEMENT concert-score EMPTY>
    }

    //-----------------------------------------------------------------------------------
    void add_page_layout()
    {
        ImoDocument* pDoc = m_pScore->get_document();
        ImoPageInfo* pInfo = pDoc->get_page_info();

	    if (!pInfo->is_page_layout_modified())
	        return;

        start_element_no_attribs("page-layout");

	    if (pInfo->is_page_size_modified())
	    {
            float value = m_pScore->logical_to_tenths( pInfo->get_page_height() );
            create_element("page-height", value);

            value = m_pScore->logical_to_tenths( pInfo->get_page_width() );
            create_element("page-width", value);
	    }

        if (pInfo->are_page_margins_modified())
        {
            bool fBoth = pInfo->get_left_margin_odd() == pInfo->get_left_margin_even()
                        && pInfo->get_right_margin_odd() == pInfo->get_right_margin_even()
                        && pInfo->get_top_margin_odd() == pInfo->get_top_margin_even()
                        && pInfo->get_bottom_margin_odd() == pInfo->get_bottom_margin_even();

            if (fBoth)
            {
                start_element("page-margins");
                add_attribute("type", "both");
                close_start_tag();
                create_element("left-margin", m_pScore->logical_to_tenths(pInfo->get_left_margin_odd()));
                create_element("right-margin", m_pScore->logical_to_tenths(pInfo->get_right_margin_odd()));
                create_element("top-margin", m_pScore->logical_to_tenths(pInfo->get_top_margin_odd()));
                create_element("bottom-margin", m_pScore->logical_to_tenths(pInfo->get_bottom_margin_odd()));
                end_element();  //page-margins
            }
            else
            {
                if (pInfo->are_even_page_margins_modified())
                {
                    start_element("page-margins");
                    add_attribute("type", "even");
                    close_start_tag();
                    create_element("left-margin", m_pScore->logical_to_tenths(pInfo->get_left_margin_even()));
                    create_element("right-margin", m_pScore->logical_to_tenths(pInfo->get_right_margin_even()));
                    create_element("top-margin", m_pScore->logical_to_tenths(pInfo->get_top_margin_even()));
                    create_element("bottom-margin", m_pScore->logical_to_tenths(pInfo->get_bottom_margin_even()));
                    end_element();  //page-margins
                }
                if (pInfo->are_odd_page_margins_modified())
                {
                    start_element("page-margins");
                    add_attribute("type", "odd");
                    create_element("left-margin", m_pScore->logical_to_tenths(pInfo->get_left_margin_odd()));
                    create_element("right-margin", m_pScore->logical_to_tenths(pInfo->get_right_margin_odd()));
                    create_element("top-margin", m_pScore->logical_to_tenths(pInfo->get_top_margin_odd()));
                    create_element("bottom-margin", m_pScore->logical_to_tenths(pInfo->get_bottom_margin_odd()));
                    end_element();  //page-margins
                }
            }
        }

        end_element();  //page-layout
    }

    //-----------------------------------------------------------------------------------
    void add_system_layout()
    {
        ImoSystemInfo* pInfo = m_pScore->get_first_system_info();
	    if (pInfo->is_system_layout_modified())
	    {
            start_element_no_attribs("system-layout");

            //system-margins
            if (pInfo->is_left_margin_modified() || pInfo->is_right_margin_modified())
            {
                start_element_no_attribs("system-margins");
                create_element("left-margin", m_pScore->logical_to_tenths(pInfo->get_left_margin()));
                create_element("right-margin", m_pScore->logical_to_tenths(pInfo->get_right_margin()));
                end_element();  //sytem-margins
            }

            //system-distance
            if (pInfo->is_system_distance_modified())
                create_element("system-distance", m_pScore->logical_to_tenths(pInfo->get_system_distance()));

            //top-system-distance
            if (pInfo->is_top_system_distance_modified())
                create_element("top-system-distance", m_pScore->logical_to_tenths(pInfo->get_top_system_distance()));

            //system-dividers
            //TODO: Not yet imported. Not supported in IM

            end_element();  //system-layout
	    }
    }

    //-----------------------------------------------------------------------------------
    void add_staff_layout()
    {
        //TODO
        //All found examples are just
        //    <staff-layout>
        //      <staff-distance>93</staff-distance>
        //    </staff-layout>
        //
        //the <staff-distance> is use to initialize the staff margin of all staves:
        //    pInstr->set_staff_margin(iStaff, m_pAnalyser->get_default_staff_distance(iStaff));
        //But in Lomse, this distance can be different for each staff and will be adjusted
        //dynamically by the vertical spacing algorithm. It is non-sense to export this
        //default staff value unless the exported score has been imported from MusicXML
        //or has been created from scratch and this value explicitly set

//        if (!m_pScore->created_by_musicxml())
//            return;
//
//        start_element_no_attribs("staff-layout");
//
//        end_element();  //staff-layout
    }

    //-----------------------------------------------------------------------------------
    void add_appearance()
    {
        //TODO dtd layout
        //<!ELEMENT appearance
        //	(line-width*, note-size*, distance*, glyph*,
        //	 other-appearance*)>

        //Example: (MozartTrio.musicxml)
        //    <appearance>
        //      <line-width type="stem">0.957</line-width>
        //      <line-width type="beam">5</line-width>
        //      <line-width type="staff">1.25</line-width>
        //      <line-width type="light barline">1.4583</line-width>
        //      <line-width type="heavy barline">5</line-width>
        //      <line-width type="leger">1.875</line-width>
        //      <line-width type="ending">1.4583</line-width>
        //      <line-width type="wedge">0.9375</line-width>
        //      <line-width type="enclosure">1.4583</line-width>
        //      <line-width type="tuplet bracket">1.4583</line-width>
        //      <note-size type="grace">50</note-size>
        //      <note-size type="cue">50</note-size>
        //      <distance type="hyphen">60</distance>
        //      <distance type="beam">8</distance>
        //    </appearance>
    }

    //-----------------------------------------------------------------------------------
    void add_music_font()
    {
        //TODO
        //<!ELEMENT music-font EMPTY>
        //<!ATTLIST music-font
        //    %font;
        //>
        //    <music-font font-family="Maestro" font-size="10.7"/>
    }

    //-----------------------------------------------------------------------------------
    void add_word_font()
    {
        //<!ELEMENT word-font EMPTY>
        //<!ATTLIST word-font
        //    %font;
        //>
        //    <word-font font-family="Times New Roman" font-size="5.3"/>
    }

    //-----------------------------------------------------------------------------------
    void add_lyric_font()
    {
        //TODO
        //<!ELEMENT lyric-font EMPTY>
        //<!ATTLIST lyric-font
        //    number NMTOKEN #IMPLIED
        //    name CDATA #IMPLIED
        //    %font;
        //>
    }

    //-----------------------------------------------------------------------------------
    void add_lyric_language()
    {
        //TODO
        //<!ELEMENT lyric-language EMPTY>
        //<!ATTLIST lyric-language
        //    number NMTOKEN #IMPLIED
        //    name CDATA #IMPLIED
        //    xml:lang CDATA #REQUIRED
        //>
    }
};


//---------------------------------------------------------------------------------------
class SlurMxlGenerator : public MxlGenerator
{
protected:
    ImoSlur* m_pSlur;
    ImoNote* m_pNote;

public:
    SlurMxlGenerator(ImoObj* pSlur, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pSlur( static_cast<ImoSlur*>(pSlur) )
        , m_pNote( static_cast<ImoNote*>(pParent) )
    {
    }

    string generate_source() override
    {
        if (m_pNote == m_pSlur->get_start_object())
        {
            start_element_if_not_started("notations");
            start_element("slur");
            add_attribute("number", m_pExporter->start_slur_and_get_number(m_pSlur->get_id()));
            add_attribute("type", "start");

            //TODO
            //%line-type;
            //%dashed-formatting;
            add_attributes_for_position(m_pSlur);

            //%placement;
            //%orientation;
            //TODO: placement is not correctly imported and interferes with orientation
            if (m_pSlur->get_orientation() != k_orientation_default)
            {
                add_attribute("placement",
                    m_pSlur->get_orientation() == k_orientation_over ? "above" : "below");
            }

            //TODO
            //%bezier;
            //%color;
            add_attribute_optional_unique_id(m_pSlur);

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
class SoundMxlGenerator : public MxlGenerator
{
protected:
    ImoSoundChange* m_pImo;

public:
    SoundMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoSoundChange*>(pImo))
    {
    }

    string generate_source() override
    {
        end_element_if_started("attributes");
        start_element("sound");

        // attrib: tempo
        if (m_pImo->has_attributte(k_attr_tempo))
            add_attribute("tempo", m_pImo->get_float_attribute(k_attr_tempo));

        // attrib: dynamics
        if (m_pImo->has_attributte(k_attr_dynamics))
            add_attribute("dynamics", m_pImo->get_float_attribute(k_attr_dynamics));

        // attrib: dacapo %yes-no; #IMPLIED
        if (m_pImo->has_attributte(k_attr_dacapo))
            add_attribute("dacapo", m_pImo->get_bool_attribute(k_attr_dacapo) ? "yes" : "no");

        // attrib: segno
        if (m_pImo->has_attributte(k_attr_segno))
            add_attribute("segno", m_pImo->get_string_attribute(k_attr_segno));

        // attrib: dalsegno
        if (m_pImo->has_attributte(k_attr_dalsegno))
            add_attribute("dalsegno", m_pImo->get_string_attribute(k_attr_dalsegno));

        // attrib: coda
        if (m_pImo->has_attributte(k_attr_coda))
            add_attribute("coda", m_pImo->get_string_attribute(k_attr_coda));

        // attrib: tocoda
        if (m_pImo->has_attributte(k_attr_tocoda))
            add_attribute("tocoda", m_pImo->get_string_attribute(k_attr_tocoda));

        // attrib: divisions
            //TODO: note yet imported

        // attrib: forward-repeat. When used, value must be 'yes'
        if (m_pImo->has_attributte(k_attr_forward_repeat))
            add_attribute("forward-repeat", "yes");

        // attrib: fine
        if (m_pImo->has_attributte(k_attr_fine))
            add_attribute("fine", "yes");

        // attrib: %time-only;
        if (m_pImo->has_attributte(k_attr_time_only))
            add_attribute("time-only", m_pImo->get_string_attribute(k_attr_time_only));

        // attrib: pizzicato
        if (m_pImo->has_attributte(k_attr_pizzicato))
            add_attribute("pizzicato", m_pImo->get_bool_attribute(k_attr_pizzicato) ? "yes" : "no");

        //AWARE: pan and elevation attributtes were deprecated in v2.0

        //TODO: yes-no-number attributes
        // attrib: damper-pedal
        // attrib: soft-pedal %yes-no-number; #IMPLIED
        // attrib: sostenuto-pedal %yes-no-number; #IMPLIED

        add_attribute_optional_unique_id(m_pImo);

            // content

        bool fHasContent = false;
        bool fStartTagClosed = false;

        //(midi-device?, midi-instrument?, play?)*
        ImoObj::children_iterator it;
        for (it=m_pImo->begin(); it != m_pImo->end(); ++it)
        {
            if ((*it)->is_midi_info())
            {
                ImoMidiInfo* pInfo = static_cast<ImoMidiInfo*>(*it);
                fHasContent = true;
                if (!fStartTagClosed)
                    close_start_tag();
                add_source_for(pInfo);
            }
        }

        //offset
    //            if ()
    //            {
    //                fHasContent = true;
    //                if (!fStartTagClosed)
    //                    close_start_tag();
    //            }

        //close sound element
        if (fHasContent)
            end_element();
        else
            end_element(false, false);

        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class StylesMxlGenerator : public MxlGenerator
{
protected:
    ImoStyles* m_pObj;

public:
    StylesMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
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
class TransposeMxlGenerator : public MxlGenerator
{
protected:
    ImoTranspose* m_pImo;

public:
    TransposeMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoTranspose*>(pImo))
    {
    }

    string generate_source() override
    {
        start_element_if_not_started("attributes");
        start_element("transpose", m_pImo);
        add_attributes();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    //-----------------------------------------------------------------------------------
    void add_attributes()
    {
        int iStaff = m_pImo->get_applicable_staff() + 1;
        if (iStaff > 0)
            add_attribute("number", iStaff);

        add_attribute_optional_unique_id(m_pImo);

        close_start_tag();
    }

    //-----------------------------------------------------------------------------------
    void add_content()
    {
        if (m_pImo->get_diatonic() != 0)
            create_element("diatonic", m_pImo->get_diatonic());

        if (m_pImo->get_chromatic() != 0)
            create_element("chromatic", m_pImo->get_chromatic());

        if (m_pImo->get_octave_change() != 0)
            create_element("octave-change", m_pImo->get_octave_change());

        if (m_pImo->get_doubled() != 0)
            empty_element("double");
    }

};


//---------------------------------------------------------------------------------------
class TupletMxlGenerator : public MxlGenerator
{
protected:
    ImoTuplet* m_pTuplet;
    ImoNoteRest* m_pNR;

public:
    TupletMxlGenerator(ImoObj* pTuplet, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pTuplet( static_cast<ImoTuplet*>(pTuplet) )
        , m_pNR( static_cast<ImoNoteRest*>(pParent) )
    {
    }

    string generate_source() override
    {
        //TODO  attributes
        //    %line-shape;
        //    %position;
        //    %placement;
        //add_attribute_optional_unique_id(m_pTuplet);

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
class VoltaBracketMxlGenerator : public MxlGenerator
{
protected:
    ImoVoltaBracket* m_pImo;
    ImoBarline* m_pParent;

public:
    VoltaBracketMxlGenerator(ImoObj* pImo, ImoObj* pParent, MxlExporter* pExporter)
        : MxlGenerator(pExporter)
        , m_pImo( static_cast<ImoVoltaBracket*>(pImo))
        , m_pParent( static_cast<ImoBarline*>(pParent))
    {
    }

    string generate_source() override
    {
        start_element("ending", m_pImo);

        //attrib: number
        add_attribute("number", m_pImo->get_volta_number());

        //attrib: number
        if (m_pImo->get_start_barline() == m_pParent)
            add_attribute("type", "start");
        else if (m_pImo->has_final_jog())
            add_attribute("type", "stop");
        else
            add_attribute("type", "discontinue");

        //attrib: %print-object;
        add_attribute_print_object(m_pImo);

        //attrib: %print-style;
        add_attributes_for_print_style(m_pImo);
        //TODO
        //    end-length %tenths; #IMPLIED
        //    text-x %tenths; #IMPLIED
        //    text-y %tenths; #IMPLIED

        //content
        if (m_pImo->get_volta_text().empty())
            end_element(false, false);
        else
        {
            m_source << m_pImo->get_volta_text();
            close_start_tag();
        }
        return m_source.str();
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
void MxlGenerator::start_element(const string& name, ImoObj* UNUSED(pImo))
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
void MxlGenerator::create_element(const string& name, float content)
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
void MxlGenerator::add_attribute(const string& name, float value)
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
void MxlGenerator::add_source_for(ImoObj* pImo, ImoObj* pParent)
{
    m_source << m_pExporter->get_source(pImo, pParent);
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

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attributes_for_print_style_align(ImoObj* pImo)
{
    //<!ENTITY % print-style-align
    //	"%print-style;
    //	 %halign;
    //	 %valign;">

    add_attributes_for_print_style(pImo);
    //TODO
//    add_attributes_for_halign(pImo);
//    add_attribute_for_valign(pImo);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attributes_for_print_style(ImoObj* pImo)
{
    //@<!ENTITY % print-style
    //@    "%position;
    //@     %font;
    //@     %color;">

    add_attributes_for_position(pImo);
    add_attributes_for_font(pImo);
    add_attribute_color(pImo);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attributes_for_printout(ImoObj* pImo)
{
    //<!ENTITY % printout
    //	"%print-object;
    //	 print-dot     %yes-no;  #IMPLIED
    //	 %print-spacing;
    //	 print-lyric   %yes-no;  #IMPLIED">

    add_attribute_print_object(pImo);
    //TODO
//    add_attribute_print_dot(pImo);
//    add_attribute_print_spacing(pImo);
        //<!ENTITY % print-spacing
        //"print-spacing %yes-no;  #IMPLIED">
//    add_attribute_print_lyric(pImo);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attributes_for_position(ImoObj* pObj)
{
    //@<!ENTITY % position
    //@    "default-x     %tenths;    #IMPLIED
    //@     default-y     %tenths;    #IMPLIED
    //@     relative-x    %tenths;    #IMPLIED
    //@     relative-y    %tenths;    #IMPLIED">

    if (!pObj || !pObj->is_contentobj())
        return;

    ImoContentObj* pImo = static_cast<ImoContentObj*>(pObj);

    Tenths pos = pImo->get_user_ref_point_x();
    if (pos != 0.0f)
        add_attribute("default-x", pos);

    pos = pImo->get_user_ref_point_y();
    if (pos != 0.0f)
        //AWARE: positive y is up, negative y is down
        add_attribute("default-y", -pos);

    pos = pImo->get_user_location_x();
    if (pos != 0.0f)
        add_attribute("relative-x", pos);

    pos = pImo->get_user_location_y();
    if (pos != 0.0f)
        //AWARE: positive y is up, negative y is down
        add_attribute("relative-y", -pos);
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attibutes_for_text_formatting(ImoObj* pObj)
{
    //<!ENTITY % text-formatting
    //	"%justify;
    add_attributes_for_print_style_align(pObj);
    //	 %text-decoration;
    //	 %text-rotation;
    //	 %letter-spacing;
    //	 %line-height;
    //	 xml:lang CDATA #IMPLIED
    //	 xml:space (default | preserve) #IMPLIED
    //	 %text-direction;
    //	 %enclosure;">
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attributes_for_font(ImoObj* pObj)
{
    if (pObj->is_contentobj())
    {
        //TODO
        //ImoContentObj* pImo = static_cast<ImoContentObj*>(pObj);
    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attribute_color(ImoObj* UNUSED(pObj))
{
    //TODO
//    if (pObj->is_contentobj())
//    {
//        if (!static_cast<ImoContentObj*>(pObj)->is_visible())
//            add_attribute("color", "no");
//    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attribute_print_object(ImoObj* pObj)
{
    if (pObj->is_contentobj())
    {
        if (!static_cast<ImoContentObj*>(pObj)->is_visible())
            add_attribute("print-object", "no");
    }
}

//---------------------------------------------------------------------------------------
void MxlGenerator::add_attribute_optional_unique_id(ImoObj* UNUSED(pObj))
{
    //TODO: This is not supported/imported
    //<!--
    //	The optional-unique-id entity allows an element to optionally
    //	specify an ID that is unique to the entire document. This
    //	entity is not used for a required id attribute, or for an id
    //	attribute that specifies an id reference.
    //-->
    //<!ENTITY % optional-unique-id
    //	"id ID #IMPLIED">
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
    string value = m_libraryScope.get_version_long_string();
    value.append(", date ");
    value.append(m_exportTime);
    return value;
}

//---------------------------------------------------------------------------------------
string MxlExporter::get_source(ImoObj* pImo, ImoObj* pParent)
{
    MxlGenerator* pGen = new_generator(pImo, pParent);
    string source = pGen->generate_source();

    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
string MxlExporter::get_source(AScore score)
{
    if (score.is_valid())
        return get_source(score.internal_object(), nullptr);

    return string();
}

//---------------------------------------------------------------------------------------
MxlGenerator* MxlExporter::new_generator(ImoObj* pImo, ImoObj* pParent)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_arpeggio:        return LOMSE_NEW ArpeggioMxlGenerator(pImo, pParent, this);
        case k_imo_barline:         return LOMSE_NEW BarlineMxlGenerator(pImo, pParent, this);
        case k_imo_clef:            return LOMSE_NEW ClefMxlGenerator(pImo, pParent, this);
        case k_imo_direction:       return LOMSE_NEW DirectionMxlGenerator(pImo, pParent, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocMxlGenerator(pImo, pParent, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentMxlGenerator(pImo, pParent, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureMxlGenerator(pImo, pParent, this);
        case k_imo_midi_info:       return LOMSE_NEW MidiInfoMxlGenerator(pImo, pParent, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataMxlGenerator(pImo, pParent, this);
        case k_imo_note_regular:    return LOMSE_NEW NoteRestMxlGenerator(pImo, pParent, this);
        case k_imo_note_grace:      return LOMSE_NEW NoteRestMxlGenerator(pImo, pParent, this);
        case k_imo_note_cue:        return LOMSE_NEW NoteRestMxlGenerator(pImo, pParent, this);
        case k_imo_ornament:        return LOMSE_NEW OrnamentMxlGenerator(pImo, pParent, this);
        case k_imo_rest:            return LOMSE_NEW NoteRestMxlGenerator(pImo, pParent, this);
        case k_imo_score:           return LOMSE_NEW ScoreMxlGenerator(pImo, pParent, this);
        case k_imo_slur:            return LOMSE_NEW SlurMxlGenerator(pImo, pParent, this);
        case k_imo_sound_change:    return LOMSE_NEW SoundMxlGenerator(pImo, pParent, this);
        case k_imo_style:           return LOMSE_NEW DefineStyleMxlGenerator(pImo, pParent, this);
        case k_imo_styles:          return LOMSE_NEW StylesMxlGenerator(pImo, pParent, this);
        case k_imo_time_signature:  return LOMSE_NEW TimeSignatureMxlGenerator(pImo, pParent, this);
        case k_imo_transpose:       return LOMSE_NEW TransposeMxlGenerator(pImo, pParent, this);
        case k_imo_tuplet:          return LOMSE_NEW TupletMxlGenerator(pImo, pParent, this);
        case k_imo_volta_bracket:   return LOMSE_NEW VoltaBracketMxlGenerator(pImo, pParent, this);

        default:
            return new ErrorMxlGenerator(pImo, pParent, this);
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
        case k_longa:   return "long";
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


//---------------------------------------------------------------------------------------
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


}  //namespace lomse
