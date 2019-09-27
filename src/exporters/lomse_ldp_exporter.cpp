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

#include "lomse_ldp_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_logger.h"

#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
// LdpGenerator
//=======================================================================================
class LdpGenerator
{
protected:
    LdpExporter* m_pExporter;
    stringstream m_source;
    bool m_fAddSpace;           //add space when opening new element

public:
    LdpGenerator(LdpExporter* pExporter, bool fSpaceNeeded=false)
        : m_pExporter(pExporter)
        , m_fAddSpace(fSpaceNeeded)
    {
    }
    virtual ~LdpGenerator() {}

    virtual string generate_source(ImoObj* pParent=nullptr) = 0;

protected:
    void start_element(const string& name, ImoId id, bool fInNewLine=true);
    void end_element(bool fStartLine = true);
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void new_line();
    void add_source_for(ImoObj* pImo);
    void source_for_abbreviated_elements(ImoNoteRest* pNR);
    void source_for_noterest_options(ImoNoteRest* pNR);
    void source_for_staffobj_options(ImoStaffObj* pSO);
    void source_for_print_options(ImoScoreObj* pSO);
    void source_for_attachments(ImoContentObj* pSO);

    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void source_for_relobj(ImoObj* pImo, ImoObj* pParent);

    void increment_indent();
    void decrement_indent();

    void add_duration(stringstream& source, int noteType, int dots);
    void add_visible(bool fVisible);
    void add_color_if_not_black(Color color);
    void add_location_if_not_zero(Tenths x, Tenths y);
    void add_location(TPoint pt);
    void add_width_if_not_default(Tenths width, Tenths def);
    void add_style(ImoStyle* pStyle);
    void add_placement(int placement);

    inline void space_needed() { m_fAddSpace = true; }
    inline void add_space_if_needed() {
        if (m_fAddSpace)
            m_source << " ";
        m_fAddSpace = false;
    }
    inline bool is_space_needed() { return m_fAddSpace; }
};

const bool k_in_same_line = false;
const bool k_in_new_line = true;
const int k_indent_step = 3;


//=======================================================================================
// generators for specific elements
//=======================================================================================


//---------------------------------------------------------------------------------------
class xxxxxxLdpGenerator : public LdpGenerator
{
protected:
    //ImoXXXXX* m_pObj;

public:
    xxxxxxLdpGenerator(ImoObj* UNUSED(pImo), LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        //m_pObj = static_cast<ImoXXXXX*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        //start_element("xxxxx", m_pObj->get_id());
        end_element();
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class ArticulationSymbolLdpGenerator : public LdpGenerator
{
protected:
    ImoArticulationSymbol* m_pObj;

public:
    ArticulationSymbolLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoArticulationSymbol*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_articulation();
        if (m_pObj->is_accent() || m_pObj->is_stress())
            add_placement( m_pObj->get_placement() );
        source_for_base_scoreobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void start_articulation()
    {
        int type = m_pObj->get_articulation_type();
        switch( type )
        {
            //accents
            case k_articulation_accent:
                start_element("accent", m_pObj->get_id());
                return;
            case k_articulation_marccato:
                start_element("marccato", m_pObj->get_id());
                return;
            case k_articulation_staccato:
                start_element("staccato", m_pObj->get_id());
                return;
            case k_articulation_tenuto:
                start_element("tenuto", m_pObj->get_id());
                return;
            case k_articulation_mezzo_staccato:
                start_element("mezzo-staccato", m_pObj->get_id());
                return;
            case k_articulation_staccatissimo:
                start_element("staccatissimo", m_pObj->get_id());
                return;
            case k_articulation_legato_duro:
                start_element("legato-duro", m_pObj->get_id());
                return;
            case k_articulation_marccato_legato:
                start_element("marccato-legato", m_pObj->get_id());
                return;
            case k_articulation_marccato_staccato:
                start_element("marccato-staccato", m_pObj->get_id());
                return;
            case k_articulation_staccato_duro:
                start_element("staccato-duro", m_pObj->get_id());
                return;
            case k_articulation_marccato_staccatissimo:
                start_element("marccato-staccatissimo", m_pObj->get_id());
                return;
            case k_articulation_mezzo_staccatissimo:
                start_element("mezzo-staccatissimo", m_pObj->get_id());
                return;
            case k_articulation_staccatissimo_duro:
                start_element("staccatissimo-duro", m_pObj->get_id());
                return;

            //jazz pitch articulations
            case k_articulation_scoop:
                start_element("scoop", m_pObj->get_id());
                return;
            case k_articulation_plop:
                start_element("plop", m_pObj->get_id());
                return;
            case k_articulation_doit:
                start_element("doit", m_pObj->get_id());
                return;
            case k_articulation_falloff:
                start_element("fallOff", m_pObj->get_id());
                return;

            //breath marks
            case k_articulation_breath_mark:
                start_element("breathMark", m_pObj->get_id());
                switch (m_pObj->get_symbol())
                {
                    case ImoArticulationSymbol::k_breath_tick:
                        m_source << " tick";
                        return;
                    case ImoArticulationSymbol::k_breath_v:
                        m_source << " V";
                        return;
                    case ImoArticulationSymbol::k_breath_salzedo:
                        m_source << " salzedo";
                        return;
                    case ImoArticulationSymbol::k_breath_comma:
                    default:
                        return;
                }

            case k_articulation_caesura:
                start_element("caesura", m_pObj->get_id());
                switch (m_pObj->get_symbol())
                {
                    case ImoArticulationSymbol::k_caesura_thick:
                        m_source << " thick";
                        return;
                    case ImoArticulationSymbol::k_caesura_short:
                        m_source << " short";
                        return;
                    case ImoArticulationSymbol::k_caesura_curved:
                        m_source << " curved";
                        return;
                    case ImoArticulationSymbol::k_caesura_normal:
                    default:
                        return;
                }

            //stress accents
            case k_articulation_stress:
                start_element("stress", m_pObj->get_id());
                return;
            case k_articulation_unstress:
                start_element("unstress", m_pObj->get_id());
                return;

            //other in MusicXML
            case k_articulation_spiccato:
                start_element("spicato", m_pObj->get_id());
                return;

            //unexpected types: code maintenance problem
            default:
                stringstream s;
                s << "Code incoherence: articulation " << type
                  << " not expected in LDP exporter: ArticulationSymbolLdpGenerator."
                  << endl;
                LOMSE_LOG_ERROR(s.str());
                return;
        }
    }

    void add_symbol()
    {
        switch(m_pObj->get_symbol())
        {
            case ImoFermata::k_short:
                m_source << " short";
                break;
            case ImoFermata::k_long:
                m_source << " long";
                break;
            case ImoFermata::k_henze_short:
                m_source << " henze-short";
                break;
            case ImoFermata::k_henze_long:
                m_source << " henze-long";
                break;
            case ImoFermata::k_very_short:
                m_source << " very-short";
                break;
            case ImoFermata::k_very_long:
                m_source << " very-long";
                break;

            case ImoFermata::k_normal:
            default:
                return;
        }
    }

};

//---------------------------------------------------------------------------------------
//@ <barline> = (barline) | (barline <type>[middle]<staffObjOptions>* <attachments>* )
class BarlineLdpGenerator : public LdpGenerator
{
protected:
    ImoBarline* m_pObj;

public:
    BarlineLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("barline", m_pObj->get_id());
        add_barline_type_and_middle();
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_barline_type_and_middle()
    {
        int type = m_pObj->get_type();
        string name = LdpExporter::barline_type_to_ldp(type);
        if (name == "undefined")
        {
            m_source << " simple";
            stringstream s;
            s << "Invalid barline. Type=" << type;
            LOMSE_LOG_ERROR(s.str());
        }
        else
            m_source << " " << name;

        if (m_pObj->is_middle())
            m_source << " middle";

        space_needed();
    }

};

//---------------------------------------------------------------------------------------
class BeamLdpGenerator : public LdpGenerator
{
protected:
    ImoRelObj* m_pRO;
    ImoNoteRest* m_pNR;

public:
    BeamLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pRO( static_cast<ImoRelObj*>(pImo) )
        , m_pNR(nullptr)
    {
    }

    string generate_source(ImoObj* pParent=nullptr)
    {
        m_pNR = static_cast<ImoNoteRest*>( pParent );

        //skip if note in chord and not base of chord
        bool fSkip = false;
        if (pParent && pParent->is_note())
        {
            ImoNote* pNote = static_cast<ImoNote*>(pParent);
            fSkip = pNote->is_in_chord() && ! pNote->is_start_of_chord();
        }

        if (!fSkip)
        {
            if ( m_pNR == m_pRO->get_start_object() )
                return source_for_first();
            else if ( m_pNR == m_pRO->get_end_object() )
                return source_for_last();
            else
                return source_for_middle();
        }
        return m_source.str();
    }

protected:

    string source_for_first()
    {
        start_element("beam", m_pRO->get_id(), k_in_same_line);
        add_beam_number();
        add_segments_info();
        end_element(k_in_same_line);
        return m_source.str();
    }

    string source_for_middle()
    {
        return source_for_first();
    }

    string source_for_last()
    {
        return source_for_first();
    }

    void add_beam_number()
    {
        m_source << " " << m_pRO->get_id();
    }

    void add_segments_info()
    {
        m_source << " ";
        for (int i=0; i < 6; ++i)
        {
            int type = m_pNR->get_beam_type(i);
            if (type == ImoBeam::k_none)
                break;
            else if (type == ImoBeam::k_begin)
                m_source << "+";
            else if (type == ImoBeam::k_continue)
                m_source << "=";
            else if (type == ImoBeam::k_end)
                m_source << "-";
            else if (type == ImoBeam::k_forward)
                m_source << "f";
            else if (type == ImoBeam::k_backward)
                m_source << "b";
        }
    }

};

//---------------------------------------------------------------------------------------
//@ <clef> = (clef <type> [<symbolSize>] <staffObjOptions>* <attachments>* )
class ClefLdpGenerator : public LdpGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("clef", m_pObj->get_id());
        add_type();
        add_symbol_size();
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_type()
    {
        m_source << " " << LdpExporter::clef_type_to_ldp( m_pObj->get_clef_type() );
        space_needed();
    }

    void add_symbol_size()
    {
        int size = m_pObj->get_symbol_size();
        if (size != k_size_default)
        {
            start_element("symbolSize", m_pObj->get_id());
            if (size == k_size_cue)
                m_source << " cue";
            else if (size == k_size_full)
                m_source << " full";
            else
                m_source << " large";
            end_element(k_in_same_line);
        }
    }

};

//---------------------------------------------------------------------------------------
class ContentObjLdpGenerator : public LdpGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoContentObj*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        add_user_location();
        add_attachments();
        source_for_base_imobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_user_location()
    {
        Tenths ux = m_pObj->get_user_location_x();
        if (ux != 0.0f)
        {
            add_space_if_needed();
            m_source << "(dx " << LdpExporter::float_to_string(ux) << ")";
        }

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
        {
            add_space_if_needed();
            m_source << "(dy " << LdpExporter::float_to_string(uy) << ")";
        }
    }

    void add_attachments()
    {
        if (m_pObj->get_num_attachments() > 0)
        {
            ImoAttachments* pAuxObjs = m_pObj->get_attachments();
            int size = pAuxObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
                source_for_auxobj(pAO);
            }
        }
    }
};

//@--------------------------------------------------------------------------------------
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

class DefineStyleLdpGenerator : public LdpGenerator
{
protected:
    ImoStyle* m_pObj;

public:
    DefineStyleLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoStyle*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("defineStyle", k_no_imoid, k_in_new_line);
        add_name();
        add_properties();
        end_element();
        return m_source.str();
    }

protected:

    void add_name()
    {
        m_source << " \"" << m_pObj->get_name() << "\" ";
    }

    void add_properties()
    {
        string sValue;
        LUnits uValue;
        int iValue;
        float rValue;
        Color cValue;

        //font properties
        if (m_pObj->get_string_property(ImoStyle::k_font_file, &sValue))
        {
            if (!sValue.empty())     //font-file can be empty when font-name is set
                create_string_element("font-file", sValue);
        }

        if (m_pObj->get_string_property(ImoStyle::k_font_name, &sValue))
            create_string_element("font-name", sValue);

        if (m_pObj->get_float_property(ImoStyle::k_font_size, &rValue))
        {
            start_element("font-size", k_no_imoid);
            m_source << " " << rValue << "pt";
            end_element(k_in_same_line);
        }

        if (m_pObj->get_int_property(ImoStyle::k_font_style, &iValue))
            create_font_style(iValue);

        if (m_pObj->get_int_property(ImoStyle::k_font_weight, &iValue))
            create_font_weight(iValue);

        //text properties
        if (m_pObj->get_int_property(ImoStyle::k_word_spacing, &iValue))
            create_int_element("word-spacing", iValue);

        if (m_pObj->get_int_property(ImoStyle::k_text_decoration, &iValue))
            create_text_decoration(iValue);

        if (m_pObj->get_int_property(ImoStyle::k_vertical_align, &iValue))
            create_vertical_align(iValue);

        if (m_pObj->get_int_property(ImoStyle::k_text_align, &iValue))
            create_text_align(iValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_text_indent_length, &uValue))
            create_lunits_element("text-indent-length", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_word_spacing_length, &uValue))
            create_lunits_element("word-spacing-length", uValue);

        if (m_pObj->get_float_property(ImoStyle::k_line_height, &rValue))
            create_float_element("line-height", rValue);

        //color and background properties
        if (m_pObj->get_color_property(ImoStyle::k_color, &cValue))
            create_color_element("color", cValue);

        if (m_pObj->get_color_property(ImoStyle::k_background_color, &cValue))
            create_color_element("background-color", cValue);

        //border-width  properties
        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_top, &uValue))
            create_lunits_element("border-width-top", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_bottom, &uValue))
            create_lunits_element("border-width-bottom", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_left, &uValue))
            create_lunits_element("border-width-left", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_border_width_right, &uValue))
            create_lunits_element("border-width-right", uValue);

        //padding properties
        if (m_pObj->get_lunits_property(ImoStyle::k_padding_top, &uValue))
            create_lunits_element("padding-top", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_padding_bottom, &uValue))
            create_lunits_element("padding-bottom", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_padding_left, &uValue))
            create_lunits_element("padding-left", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_padding_right, &uValue))
            create_lunits_element("padding-right", uValue);

        //margin properties
        if (m_pObj->get_lunits_property(ImoStyle::k_margin_top, &uValue))
            create_lunits_element("margin-top", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_margin_bottom, &uValue))
            create_lunits_element("margin-bottom", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_margin_left, &uValue))
            create_lunits_element("margin-left", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_margin_right, &uValue))
            create_lunits_element("margin-right", uValue);

        //size properties
        if (m_pObj->get_lunits_property(ImoStyle::k_width, &uValue))
            create_lunits_element("width", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_height, &uValue))
            create_lunits_element("height", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_min_width, &uValue))
            create_lunits_element("min-width", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_min_height, &uValue))
            create_lunits_element("min-height", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_max_width, &uValue))
            create_lunits_element("max-width", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_max_height, &uValue))
            create_lunits_element("max-height", uValue);

        //table properties
        if (m_pObj->get_lunits_property(ImoStyle::k_table_col_width, &uValue))
            create_lunits_element("table-col-width", uValue);
    }

    void create_string_element(const string& tag, const string& value)
    {
        start_element(tag, k_no_imoid);
        m_source << " \"" << value << "\"";
        end_element(k_in_same_line);
    }

    void create_float_element(const string& tag, float value)
    {
        start_element(tag, k_no_imoid);
        m_source << " " << value;
        end_element(k_in_same_line);
    }

    void create_lunits_element(const string& tag, LUnits value)
    {
        start_element(tag, k_no_imoid);
        m_source << " " << value;
        end_element(k_in_same_line);
    }

    void create_int_element(const string& tag, int value)
    {
        start_element(tag, k_no_imoid);
        m_source << " " << value;
        end_element(k_in_same_line);
    }

    void create_color_element(const string& tag, Color color)
    {
        start_element(tag, k_no_imoid);
        m_source << " " << LdpExporter::color_to_ldp(color);
        end_element(k_in_same_line);
    }

    void create_font_style(int value)
    {
        start_element("font-style", k_no_imoid);

        if (value == ImoStyle::k_font_style_normal)
            m_source << " normal";
        else if (value == ImoStyle::k_font_style_italic)
            m_source << " italic";
        else
            m_source << " invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_font_weight(int value)
    {
        start_element("font-weight", k_no_imoid);

        if (value == ImoStyle::k_font_weight_normal)
            m_source << " normal";
        else if (value == ImoStyle::k_font_weight_bold)
            m_source << " bold";
        else
            m_source << " invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_text_decoration(int value)
    {
        start_element("text-decoration", k_no_imoid);

        if (value == ImoStyle::k_decoration_none)
            m_source << " none";
        else if (value == ImoStyle::k_decoration_underline)
            m_source << " underline";
        else if (value == ImoStyle::k_decoration_overline)
            m_source << " overline";
        else if (value == ImoStyle::k_decoration_line_through)
            m_source << " line-through";
        else
            m_source << " invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_vertical_align(int value)
    {
        start_element("vertical-align", k_no_imoid);

        if (value == ImoStyle::k_valign_baseline)
            m_source << " baseline";
        else if (value == ImoStyle::k_valign_sub)
            m_source << " sub";
        else if (value == ImoStyle::k_valign_super)
            m_source << " super";
        else if (value == ImoStyle::k_valign_top)
            m_source << " top";
        else if (value == ImoStyle::k_valign_text_top)
            m_source << " text-top";
        else if (value == ImoStyle::k_valign_middle)
            m_source << " middle";
        else if (value == ImoStyle::k_valign_bottom)
            m_source << " bottom";
        else if (value == ImoStyle::k_valign_text_bottom)
            m_source << " text-bottom";
        else
            m_source << " invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_text_align(int value)
    {
        start_element("text-align", k_no_imoid);

        if (value == ImoStyle::k_align_left)
            m_source << " left";
        else if (value == ImoStyle::k_align_right)
            m_source << " right";
        else if (value == ImoStyle::k_align_center)
            m_source << " center";
        else if (value == ImoStyle::k_align_justify)
            m_source << " justify";
        else
            m_source << " invalid value " << value;

        end_element(k_in_same_line);
    }

};

//---------------------------------------------------------------------------------------
class DirectionLdpGenerator : public LdpGenerator
{
protected:
    ImoDirection* m_pObj;

public:
    DirectionLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDirection*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        if (m_pObj->has_attachments() || m_pObj->get_num_relations() > 0)
            start_element("dir", m_pObj->get_id());
        else if (m_pObj->get_width() > 0.0f)
            start_element("spacer", m_pObj->get_id());
        else
            return string("");

        add_space_width();
        add_spanners();
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_space_width()
    {
        m_source << " " << m_pObj->get_width();
        space_needed();
    }

    void add_spanners()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                source_for_relobj(pRO, m_pObj);
            }
        }
    }

};

//---------------------------------------------------------------------------------------
class DynamicsLdpGenerator : public LdpGenerator
{
protected:
    ImoDynamicsMark* m_pObj;

public:
    DynamicsLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDynamicsMark*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("dyn", m_pObj->get_id());
        add_dynamics_string();
        space_needed();
        add_placement( m_pObj->get_placement() );
        source_for_base_scoreobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_dynamics_string()
    {
        m_source << " \"" << m_pObj->get_mark_type() << "\"";
    }
};


//---------------------------------------------------------------------------------------
class ErrorLdpGenerator : public LdpGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("TODO: ", m_pImo->get_id());
        m_source << " No LdpGenerator for Imo. Imo name=" << m_pImo->get_name()
                 << ", Imo type=" << m_pImo->get_obj_type()
                 << ", id=" << m_pImo->get_id();
        end_element(k_in_same_line);
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class FermataLdpGenerator : public LdpGenerator
{
protected:
    ImoFermata* m_pObj;

public:
    FermataLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoFermata*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("fermata", m_pObj->get_id());
        add_symbol();
        add_placement( m_pObj->get_placement() );
        source_for_base_scoreobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:
    void add_symbol()
    {
        switch(m_pObj->get_symbol())
        {
            case ImoFermata::k_short:
                m_source << " short";
                break;
            case ImoFermata::k_long:
                m_source << " long";
                break;
            case ImoFermata::k_henze_short:
                m_source << " henze-short";
                break;
            case ImoFermata::k_henze_long:
                m_source << " henze-long";
                break;
            case ImoFermata::k_very_short:
                m_source << " very-short";
                break;
            case ImoFermata::k_very_long:
                m_source << " very-long";
                break;

            case ImoFermata::k_normal:
            default:
                return;
        }
    }

};

//---------------------------------------------------------------------------------------
class ImObjLdpGenerator : public LdpGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = pImo;
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class GoBackFwdLdpGenerator : public LdpGenerator
{
protected:
    ImoGoBackFwd* m_pObj;

public:
    GoBackFwdLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoGoBackFwd*>(pImo);
    }

    //TODO: This exporter must generate 2.0 code. Therefore, it is invalid to generate
    // goBack. Instead must convert it to 2.0
    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        empty_line();
        bool fFwd = m_pObj->is_forward();
        start_element((fFwd ? "goFwd" : "goBack"), m_pObj->get_id());
        add_time(fFwd);
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_time(bool fFwd)
    {
        if (m_pObj->is_to_start())
            m_source << " start";
        else if (m_pObj->is_to_end())
            m_source << " end";
        else
        {
            if (fFwd)
                m_source << " " <<  m_pObj->get_time_shift();
            else
                m_source << " " << - m_pObj->get_time_shift();

        }
    }
};

//---------------------------------------------------------------------------------------
class InstrumentLdpGenerator : public LdpGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoInstrument*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("instrument", m_pObj->get_id());
        add_part_id();
        add_name_abbreviation();
        add_staves_info();
        add_sound_info();
        add_music_data();
        end_element();
        return m_source.str();
    }

protected:

    void add_staves_info()
    {
        start_element("staves", k_no_imoid);
        int staves = m_pObj->get_num_staves();
        m_source << " " << staves;
        end_element(k_in_same_line);

        for (int i=1; i <= staves; ++i)
        {
            ImoStaffInfo* pStaff = m_pObj->get_staff(i-1);
            if (!has_default_values(pStaff))
            {
                open_staff_element(i);
                add_staff_type(pStaff);
                add_staff_lines(pStaff);
                add_staff_spacing(pStaff);
                add_staff_distance(pStaff);
                add_line_thickness(pStaff);
                close_staff_element();
            }
        }
    }

    void add_sound_info()
    {
        if (m_pObj->get_num_sounds() > 0)
        {
            ImoSoundInfo* pInfo = m_pObj->get_sound_info(0);
            ImoMidiInfo* pMidi = pInfo->get_midi_info();
            int instr = pMidi->get_midi_program();
            int channel = pMidi->get_midi_channel();
            if (instr != 0 || channel != 0)
            {
                start_element("infoMIDI", k_no_imoid);
                m_source << " " << instr << " " << channel;
                end_element(k_in_same_line);
            }
        }
    }

    void add_part_id()
    {
        string partId = m_pObj->get_instr_id();
        if (!partId.empty())
            m_source << " " << partId;
        space_needed();
    }

    void add_name_abbreviation()
    {
        //TODO: export style, location
        if (m_pObj->has_name())
        {
            start_element("name", k_no_imoid);
            ImoScoreText& txt = m_pObj->get_name();
            m_source << " \"" << txt.get_text() << "\"";
            space_needed();
            add_style( txt.get_style() );
            end_element(k_in_same_line);
        }
        if (m_pObj->has_abbrev())
        {
            start_element("abbrev", k_no_imoid);
            ImoScoreText& txt = m_pObj->get_abbrev();
            m_source << " \"" << txt.get_text() << "\"";
            space_needed();
            add_style( txt.get_style() );
            end_element(k_in_same_line);
        }
    }

    void add_music_data()
    {
        ImoObj* pImo = m_pObj->get_musicdata();
        if (pImo)
            add_source_for(pImo);
    }

    bool has_default_values(ImoStaffInfo* pStaff)
    {
        return pStaff->get_staff_type() == ImoStaffInfo::k_staff_regular
            && pStaff->get_num_lines() == 5
            && pStaff->get_line_spacing() == 180.0f
            && pStaff->get_line_thickness() == 15.0f
            && pStaff->get_staff_margin() == 1000.0f
            ;
    }

    void add_staff_type(ImoStaffInfo* pInfo)
    {
        // (staffType { ossia | cue | editorial | regular | alternate } )

        int type = pInfo->get_staff_type();
        start_element("staffType", k_no_imoid);
        switch (type)
        {
            case ImoStaffInfo::k_staff_ossia:       m_source << " ossia";        break;
            case ImoStaffInfo::k_staff_cue:         m_source << " cue";          break;
            case ImoStaffInfo::k_staff_editorial:   m_source << " editorial";    break;
            case ImoStaffInfo::k_staff_regular:     m_source << " regular";      break;
            case ImoStaffInfo::k_staff_alternate:   m_source << " alternate";    break;
            default:
                m_source << " regular";
                stringstream ss;
                ss << "Invalid staff type " << type;
                LOMSE_LOG_ERROR(ss.str());
        }
        end_element(k_in_same_line);
    }

    void add_staff_lines(ImoStaffInfo* pStaff)
    {
        start_element("staffLines", k_no_imoid);
        m_source << " " << pStaff->get_num_lines();
        end_element(k_in_same_line);
    }

    void add_staff_spacing(ImoStaffInfo* pStaff)
    {
        start_element("staffSpacing", k_no_imoid);
        m_source << " " << pStaff->get_line_spacing();
        end_element(k_in_same_line);
    }

    void add_staff_distance(ImoStaffInfo* pStaff)
    {
        start_element("staffDistance", k_no_imoid);
        m_source << " " << pStaff->get_staff_margin();
        end_element(k_in_same_line);
    }

    void add_line_thickness(ImoStaffInfo* pStaff)
    {
        start_element("lineThickness", k_no_imoid);
        m_source << " " << pStaff->get_line_thickness();
        end_element(k_in_same_line);
    }

    void open_staff_element(int i)
    {
        start_element("staff", k_no_imoid);
        m_source << " " << i << " ";
    }

    void close_staff_element()
    {
        end_element();
    }

};


//---------------------------------------------------------------------------------------
//@ <key> = (key <type> <staffobjOptions>* <attachments>* )
class KeySignatureLdpGenerator : public LdpGenerator
{
protected:
    ImoKeySignature* m_pObj;

public:
    KeySignatureLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoKeySignature*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("key", m_pObj->get_id());

        add_key_type();
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_key_type()
    {
        m_source << " " << LdpExporter::key_type_to_ldp( m_pObj->get_key_type() );
        space_needed();
    }

};


//---------------------------------------------------------------------------------------
class LenmusdocLdpGenerator : public LdpGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("lenmusdoc", m_pObj->get_id());
        m_source << " ";
        add_version();
        add_comment();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    void add_version()
    {
        start_element("vers", k_no_imoid);
        m_source << " " << m_pObj->get_version();
        end_element(k_in_same_line);
    }

    void add_comment()
    {
        if (!m_pExporter->get_remove_newlines())
        {
            start_comment();
            m_source << "LDP file generated by Lomse";
            string& version = m_pExporter->get_library_version();
            if (!version.empty())
                m_source << ", version " << version;
            m_source << ". Date: "
                     << to_simple_string(chrono::system_clock::now());
            end_comment();
        }
    }

    void add_content()
    {
        ImoContent* pContent = m_pObj->get_content();
        start_element("content", pContent->get_id());
        int numItems = m_pObj->get_num_content_items();
        for (int i=0; i < numItems; i++)
        {
            m_source << " ";
            add_source_for( m_pObj->get_content_item(i) );
        }
        end_element();
    }

};


//---------------------------------------------------------------------------------------
class LyricLdpGenerator : public LdpGenerator
{
protected:
    ImoLyric* m_pObj;

public:
    LyricLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoLyric*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("lyric", m_pObj->get_id());
        add_lyric_number();
        add_lyric_text();
        add_style( m_pObj->get_style() );
        add_placement( m_pObj->get_placement() );
        source_for_base_scoreobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_lyric_number()
    {
        m_source << " " << m_pObj->get_number();
    }

    void add_lyric_text()
    {
        int numItems = m_pObj->get_num_text_items();
        for (int i=0; i < numItems; ++i)
        {
            ImoLyricsTextInfo* pText = m_pObj->get_text_item(i);
            m_source << " \"" << pText->get_syllable_text() << "\"";
        }

        if (m_pObj->has_hyphenation())
            m_source << " -";

        if (m_pObj->has_melisma())
        {
            m_source << " ";
            start_element("melisma", k_no_imoid, k_in_same_line);
            end_element(k_in_same_line);
        }
    }

};


//---------------------------------------------------------------------------------------
class MusicDataLdpGenerator : public LdpGenerator
{
protected:
    ImoMusicData* m_pObj;
    ImoScore* m_pScore;

public:
    MusicDataLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_curMeasure(0)
        , m_iInstr(0)
        , m_fNewMeasure(false)
        , m_pColStaffObjs(nullptr)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
        m_pScore = pExporter->get_current_score();
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("musicData", m_pObj->get_id());
        space_needed();
        add_staffobjs();
        end_element();
        return m_source.str();
    }

protected:

    int m_curMeasure;
    int m_iInstr;
    bool m_fNewMeasure;
    ColStaffObjs* m_pColStaffObjs;
    vector<TimeUnits> m_rCurTime;

    //-----------------------------------------------------------------------------------
    void add_staffobjs()
    {
        ImoInstrument* pInstr = m_pObj->get_instrument();
        m_iInstr = m_pScore->get_instr_number_for(pInstr);
        m_pColStaffObjs = m_pScore->get_staffobjs_table();

        if (m_pColStaffObjs->num_entries() < 1)
            return;

        initializations();

        ColStaffObjsIterator it = m_pColStaffObjs->begin();
        while (it != m_pColStaffObjs->end())
        {
            ColStaffObjsEntry* pEntry = *it;
            if (pEntry->num_instrument() == m_iInstr)
            {
                if (m_fNewMeasure)
                    add_measure_comment();

                ImoStaffObj* pSO = pEntry->imo_object();
                int voice = determine_voice(pSO);

                //add goFwd if necessary (for 1.x imported scores)
                if (voice > 0 && is_lower_time(m_rCurTime[voice], pEntry->time()) )
                {
                    TimeUnits shift = pEntry->time() - m_rCurTime[voice];
                    int staff = pSO->get_staff() + 1;

                    start_element("goFwd", k_no_imoid);
                    m_source << " " << shift << " v" << voice << " p" << staff;
                    end_element(k_in_same_line);

                    m_rCurTime[voice] = pEntry->time();
                }

                //update time counters with this pSO duration
                TimeUnits duration = pSO->get_duration();
                if (pSO->is_note())
                {
                    ImoNote* pNote = static_cast<ImoNote*>(pSO);
                    if (pNote->is_in_chord() && !pNote->is_end_of_chord())
                        duration = 0.0;
                }
                m_rCurTime[voice] += duration;

                //update max reached time
                m_rCurTime[0] = max(m_rCurTime[0], pEntry->time());

                //add source for this staff obj, but
                //skip key and time signatures not in staff 0 (duplicates)
                if (!((pSO->is_key_signature() || pSO->is_time_signature())
                      && pEntry->staff() != 0) )
                {
                    add_source_for(pSO);
                }

                //finish measure
                if (pSO->is_barline())
                {
                    m_fNewMeasure = true;
                    ++m_curMeasure;
                    m_rCurTime.assign(k_max_voices, pEntry->time());
                }
            }
            ++it;
        }
    }

    //-----------------------------------------------------------------------------------
    void initializations()
    {
        m_fNewMeasure = true;
        m_curMeasure = 0;
        m_rCurTime.assign(k_max_voices, 0.0);
    }

    //-----------------------------------------------------------------------------------
    int determine_voice(ImoStaffObj* pSO)
    {
        if (pSO->is_note_rest())
        {
            ImoNoteRest* pNR = static_cast<ImoNoteRest*>(pSO);
            return pNR->get_voice();
        }
        else
            return 0;
    }

    //-----------------------------------------------------------------------------------
    void add_measure_comment()
    {
        if (!m_pExporter->get_remove_newlines())
        {
            empty_line();
            start_comment();
            m_source << "measure " << (m_curMeasure + 1);
            end_comment();
        }
        m_fNewMeasure = false;
    }


#if 0  //LDP version 1.7

    enum { k_max_num_lines = 16, };

    void add_staffobjs()
    {
        ImoInstrument* pInstr = m_pObj->get_instrument();
        m_iInstr = m_pScore->get_instr_number_for(pInstr);
        m_pColStaffObjs = m_pScore->get_staffobjs_table();

        if (m_pColStaffObjs->num_entries() < 1)
            return;

        m_lines.reserve(k_max_num_lines);
        m_rCurTime = 0.0;
        ColStaffObjsIterator it = m_pColStaffObjs->begin();
        while (it != m_pColStaffObjs->end())
        {
            m_itStartOfMeasure = it;
            determine_how_many_lines_in_current_measure();
            save_start_time_and_add_measure_comment();
            add_non_timed_at_start_of_measure();
            add_source_for_this_measure();
            add_source_for_barline();
            it = m_itEndOfMeasure;
        }
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
                   && !((*it)->imo_object()->is_barline()
                        && (*it)->num_instrument() == m_iInstr) )
            {
                m_lines[ (*it)->line() ] = 1;
                ++it;
            }
        }

        m_itEndOfMeasure = it;
    }

    //-----------------------------------------------------------------------------------
    void save_start_time_and_add_measure_comment()
    {
        if (m_itStartOfMeasure != m_pColStaffObjs->end())
        {
            m_rStartTime = (*m_itStartOfMeasure)->time();

            if (!m_pExporter->get_remove_newlines())
            {
                empty_line();
                start_comment();
                m_source << "measure " << (m_curMeasure + 1);
                end_comment();
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_non_timed_at_start_of_measure()
    {
        ColStaffObjsIterator it;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            if ((*it)->num_instrument() == m_iInstr)
            {
                ImoStaffObj* pImo = (*it)->imo_object();
                if (is_greater_time(pImo->get_duration(), 0.0))
                {
                    m_itStartOfMeasure = it;
                    break;
                }
                if ((*it)->measure() == m_curMeasure)
                {
                    //skip key and time signatures not in staff 0 (duplicates)
                    if (!((pImo->is_key_signature() || pImo->is_time_signature())
                          && (*it)->staff() != 0) )
                    {
                        add_source_for(pImo);
                    }
                }
            }
        }
        m_itStartOfMeasure = it;
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_this_measure()
    {
        for (int i=0; i < k_max_num_lines; ++i)
        {
            if (m_lines[i] != 0)
                add_source_for_line(i);
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_barline()
    {
        ColStaffObjsIterator it = m_itEndOfMeasure;
        if (it != m_pColStaffObjs->end() && (*it)->imo_object()->is_barline()
            && (*it)->num_instrument() == m_iInstr)
        {
            ImoStaffObj* pImo = (*it)->imo_object();
            add_go_fwd_back_if_needed( (*it)->time() );
            add_source_for(pImo);
            ++m_itEndOfMeasure;
        }
    }

    //-----------------------------------------------------------------------------------
    void add_source_for_line(int line)
    {
        ColStaffObjsIterator it;
        for (it = m_itStartOfMeasure; it != m_itEndOfMeasure; ++it)
        {
            ColStaffObjsEntry* pEntry = *it;
            if (pEntry->num_instrument() == m_iInstr && pEntry->line() == line)
            {
                ImoStaffObj* pImo = pEntry->imo_object();
                if (line == 0 || !(pImo->is_key_signature() || pImo->is_time_signature()) )
                {
                    add_go_fwd_back_if_needed( pEntry->time() );
                    add_source_for(pImo);
                    m_rCurTime = pEntry->time() + pImo->get_duration();
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------
    void add_go_fwd_back_if_needed(TimeUnits time)
    {
        if (m_pExporter->is_processing_chord())
            return;

        if (!is_equal_time(time, m_rCurTime))
        {
            TimeUnits shift = m_rCurTime - time;
            if (shift > 0.0)
            {
                start_element("goBack", -1L);
                if (is_equal_time(time, m_rStartTime))
                    m_source << "start";
                else
                    m_source << shift;
            }
            else
            {
                start_element("goFwd", -1L);
                m_source << (-shift);
            }
            end_element(k_in_same_line);
        }
    }

    int m_curMeasure;
    int m_iInstr;
    vector<int> m_lines;
    ColStaffObjs* m_pColStaffObjs;
    ColStaffObjsIterator m_itStartOfMeasure;
    ColStaffObjsIterator m_itEndOfMeasure;
    TimeUnits m_rCurTime;
    TimeUnits m_rStartTime;

#endif  //LDP version 1.7

//    //Old code exploring ImoTree
//    void add_staffobjs()
//    {
//        ImoObj::children_iterator it = m_pObj->begin();
//        bool fNewMeasure = true;
//        int nMeasure = 1;
//        while (it != m_pObj->end())
//        {
//            if (fNewMeasure)
//            {
//                if (!m_pExporter->get_remove_newlines())
//                {
//                    empty_line();
//                    start_comment();
//                    m_source << "measure " << nMeasure++;
//                    end_comment();
//                }
//                fNewMeasure = false;
//            }
//
//            add_source_for(*it);
//            fNewMeasure = (*it)->is_barline();
//            ++it;
//        }

};

//---------------------------------------------------------------------------------------
//@ <metronome> = (metronome { <NoteType><TicksPerMinute> | <NoteType><NoteType> |
//@                            <TicksPerMinute> }
//@                          [parenthesis][<printOptions>*] )
class MetronomeLdpGenerator : public LdpGenerator
{
protected:
    ImoMetronomeMark* m_pImo;

public:
    MetronomeLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pImo = static_cast<ImoMetronomeMark*>( pImo );
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("metronome", m_pImo->get_id());
        add_marks();
        add_parenthesis();
        source_for_print_options(m_pImo);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_marks()
    {
        int type = m_pImo->get_mark_type();
        switch (type)
        {
            case ImoMetronomeMark::k_note_value:
                add_duration(m_source, m_pImo->get_left_note_type(),
                             m_pImo->get_left_dots());
                m_source << " ";
                m_source << m_pImo->get_ticks_per_minute();
                break;
            case ImoMetronomeMark::k_note_note:
                add_duration(m_source, m_pImo->get_left_note_type(),
                             m_pImo->get_left_dots());
                add_duration(m_source, m_pImo->get_right_note_type(),
                             m_pImo->get_right_dots());
                break;
            case ImoMetronomeMark::k_value:
                m_source << " " << m_pImo->get_ticks_per_minute();
                break;
            default:
            {
                m_source << " 60";
                stringstream s;
                s << "Invalid type. Value=" << type;
                LOMSE_LOG_ERROR(s.str());
            }
        }
        space_needed();
    }

    void add_parenthesis()
    {
        if (m_pImo->has_parenthesis())
        {
            m_source << " parenthesis";
            space_needed();
        }
    }

};

//---------------------------------------------------------------------------------------
class NoteLdpGenerator : public LdpGenerator
{
protected:
    ImoNote* m_pObj;

public:
    NoteLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoNote*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        if (m_pObj->is_start_of_chord())
        {
            start_element("chord", k_no_imoid);
            m_source << " ";
            m_pExporter->set_processing_chord(true);
        }

        start_element("n", m_pObj->get_id());
        m_source << " ";
        add_pitch();
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_abbreviated_elements(m_pObj);
        add_note_options();
        add_time_modifier();
        source_for_noterest_options(m_pObj);
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);

        if (m_pObj->is_end_of_chord())
        {
            end_element();
            m_pExporter->set_processing_chord(false);
        }

        return m_source.str();
    }

protected:

    void add_pitch()
    {
        static const string sNoteName[7] = { "c",  "d", "e", "f", "g", "a", "b" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pObj->get_step() == k_no_pitch)
        {
            m_source << "*";
            return;
        }

        EAccidentals acc = m_pObj->get_notated_accidentals();
        if (acc != k_no_accidentals)
            m_source << LdpExporter::accidentals_to_string(acc);

        // coverity[check_return]
        m_source << sNoteName[m_pObj->get_step()];
        m_source << sOctave[m_pObj->get_octave()];
    }

    void add_note_options()
    {
        // <tie> | <stem> | <slur> | <lyric>
        add_stem();
        add_tie_and_slurs();
        add_lyrics();
    }

    void add_stem()
    {
        int stem = m_pObj->get_stem_direction();
        switch(stem)
        {
            case k_stem_default:
                break;
            case k_stem_up:
                add_space_if_needed();
                m_source << "(stem up)";
                break;
            case k_stem_down:
                add_space_if_needed();
                m_source << "(stem down)";
                break;
            case k_stem_none:
                add_space_if_needed();
                m_source << "(stem none)";
                break;
            case k_stem_double:
                add_space_if_needed();
                m_source << "(stem double)";
                break;
            default:
            {
                stringstream s;
                s << " Invalid stem. Value=" << stem;
                LOMSE_LOG_ERROR(s.str());
            }
        }
    }

    void add_tie_and_slurs()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (pRO->is_tie() || pRO->is_slur() )
                {
                    source_for_relobj(pRO, m_pObj);
                }
            }
        }
    }

    void add_lyrics()
    {
        if (m_pObj->get_num_attachments() > 0)
        {
            ImoAttachments* pAuxObjs = m_pObj->get_attachments();
            int size = pAuxObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
                if (pAO->is_lyric())
                {
                    add_space_if_needed();
                    m_source << m_pExporter->get_source(pAO);
                }
            }
        }
    }

    void add_time_modifier()
    {
        if (m_pObj->get_time_modifier_top() != 1
            || m_pObj->get_time_modifier_bottom() != 1)
        {
            start_element("tm", k_no_imoid, k_in_same_line);
            m_source << " " << m_pObj->get_time_modifier_top()
                     << " "<< m_pObj->get_time_modifier_bottom();
            end_element(k_in_same_line);
        }
    }

};

//---------------------------------------------------------------------------------------
//@ <printOptions> = { [<visible>] [<location>] [<color>] }
class PrintOptionsLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    PrintOptionsLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        add_user_location();
        add_visible( m_pObj->is_visible() );
        add_color_if_not_black( m_pObj->get_color() );
        return m_source.str();
    }

protected:

    void add_user_location()
    {
        Tenths ux = m_pObj->get_user_location_x();
        if (ux != 0.0f)
        {
            add_space_if_needed();
            m_source << "(dx " << LdpExporter::float_to_string(ux) << ")";
        }

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
        {
            add_space_if_needed();
            m_source << "(dy " << LdpExporter::float_to_string(uy) << ")";
        }
    }
};

//---------------------------------------------------------------------------------------
class RestLdpGenerator : public LdpGenerator
{
protected:
    ImoRest* m_pObj;

public:
    RestLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoRest*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        if (is_rest())
            generate_rest();
        else
            generate_go_fwd();

        return m_source.str();
    }

protected:

    bool is_rest()
    {
        return m_pObj->is_visible()
            || m_pObj->has_attachments()
            || m_pObj->is_beamed();
    }

    void add_voice()
    {
        m_source << " v" << m_pObj->get_voice();
        space_needed();
    }

    void generate_rest()
    {
        //@ <rest> = (r <duration> <abbreviatedElements>* <noteRestOptions>*
        //@             <staffObjOptions>* <attachments>* )
        start_element("r", m_pObj->get_id());
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_abbreviated_elements(m_pObj);
        source_for_noterest_options(m_pObj);
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
    }

    void generate_go_fwd()
    {
        start_element("goFwd", m_pObj->get_id());
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_abbreviated_elements(m_pObj);
        end_element(k_in_same_line);
    }

};

//---------------------------------------------------------------------------------------
class ScoreLineLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreLine* m_pObj;

public:
    ScoreLineLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreLine*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("line", m_pObj->get_id());
        add_start_point();
        add_end_point();
        add_width_if_not_default( m_pObj->get_line_width(), 1.0f);
        add_color_if_not_black( m_pObj->get_color() );
        add_line_style();
        add_cap("lineCapStart", m_pObj->get_start_cap());
        add_cap("lineCapEnd", m_pObj->get_end_cap());
        end_element();
        return m_source.str();
    }

protected:

    void add_start_point()
    {
        start_element("startPoint", k_no_imoid, k_in_same_line);
        add_location(m_pObj->get_start_point());
        end_element(k_in_same_line);
    }

    void add_end_point()
    {
        start_element("endPoint", k_no_imoid, k_in_same_line);
        add_location(m_pObj->get_end_point());
        end_element(k_in_same_line);
    }

    void add_line_style()
    {
        int type = m_pObj->get_line_style();
        if (type == k_line_none)
            return;

        start_element("lineStyle", k_no_imoid, k_in_same_line);
        switch(type)
        {
            case k_line_solid:
                m_source << "solid";
                break;
            case k_line_long_dash:
                m_source << "longDash";
                break;
            case k_line_short_dash:
                m_source << "shortDash";
                break;
            case k_line_dot:
                m_source << "dot";
                break;
            case k_line_dot_dash:
                m_source << "dotDash";
                break;
            default:
            {
                m_source << "solid";
                stringstream s;
                s << "Invalid line style. Value=" << type;
                LOMSE_LOG_ERROR(s.str());
            }
        }
        end_element(k_in_same_line);
    }

    void add_cap(const string& tag, int type)
    {
        if (type == k_cap_none)
            return;

        start_element(tag, k_no_imoid, k_in_same_line);
        switch(type)
        {
            case k_cap_arrowhead:
                m_source << "arrowhead";
                break;
            case k_cap_arrowtail:
                m_source << "arrowtail";
                break;
            case k_cap_circle:
                m_source << "circle";
                break;
            case k_cap_square:
                m_source << "square";
                break;
            case k_cap_diamond:
                m_source << "diamond";
                break;
            default:
            {
                m_source << "arrowhead";
                stringstream s;
                s << "Invalid line cap. Value=" << type;
                LOMSE_LOG_ERROR(s.str());
            }
        }
        end_element(k_in_same_line);
    }

};

//---------------------------------------------------------------------------------------
class ScoreObjLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        add_visible( m_pObj->is_visible() );
        add_color_if_not_black( m_pObj->get_color() );
        source_for_base_contentobj(m_pObj);
        return m_source.str();
    }

};

//---------------------------------------------------------------------------------------
class ScoreTextLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreText* m_pObj;

public:
    ScoreTextLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreText*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("text", m_pObj->get_id());
        add_text();
        add_style( m_pObj->get_style() );
        add_location_if_not_zero(m_pObj->get_user_location_x(),
                                 m_pObj->get_user_location_y());
        end_element();
        return m_source.str();
    }

protected:

    void add_text()
    {
        m_source << " \"" << m_pObj->get_text() << "\"";
        space_needed();
    }

};

//---------------------------------------------------------------------------------------
class SlurLdpGenerator : public LdpGenerator
{
protected:
    ImoSlur* m_pObj;
    ImoNote* m_pNote;

public:
    SlurLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pObj( static_cast<ImoSlur*>(pImo) )
        , m_pNote(nullptr)
    {
    }

    string generate_source(ImoObj* pParent =nullptr)
    {
        m_pNote = static_cast<ImoNote*>( pParent );

        start_element("slur", m_pObj->get_id());
        add_slur_number();

        bool fStart = (m_pNote == m_pObj->get_start_note());
        add_slur_type(fStart);

        ImoBezierInfo* pInfo = (fStart ? m_pObj->get_start_bezier()
                                       : m_pObj->get_stop_bezier() );
        add_bezier_info(pInfo);

        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_slur_number()
    {
        m_source << " " << m_pObj->get_slur_number();
    }

    void add_slur_type(bool fStart)
    {
        m_source << (fStart ? " start" : " stop");
    }

    void add_bezier_info(ImoBezierInfo* pInfo)
    {
        if (pInfo)
        {
            static string sPointNames[4] = { "start", "end", "ctrol1", "ctrol2" };

            bool fElementStarted = false;
            for (int i=0; i < 4; i++)
            {
                TPoint& pt = pInfo->get_point(i);

                if (pt.x != 0.0f || pt.y != 0.0f)
                {
                    if (!fElementStarted)
                    {
                        start_element("bezier", k_no_imoid);
                        fElementStarted = true;
                    }

                    if (pt.x != 0.0f)
                    {
                        start_element( sPointNames[i] + "-x", k_no_imoid, k_in_same_line);
                        m_source << " " << pt.x;
                        end_element(k_in_same_line);
                    }
                    if (pt.y != 0.0f)
                    {
                        start_element( sPointNames[i] + "-y", k_no_imoid, k_in_same_line);
                        m_source << " " << pt.y;
                        end_element(k_in_same_line);
                    }
                }
            }
            if (fElementStarted)
                end_element();
        }
    }
};

//---------------------------------------------------------------------------------------
class StaffObjLdpGenerator : public LdpGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        add_staff_num();
        add_relobjs();
        source_for_base_scoreobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_staff_num()
    {
        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
            && !m_pObj->is_time_signature()
            && !m_pObj->is_barline()
            && !m_pObj->is_go_back_fwd()
            && !m_pObj->is_note_rest() )
        {
#if 1       // 1= px  0- (staffNum x)
            m_source << " p" << (m_pObj->get_staff() + 1);
            space_needed();
#else
            m_source << " ";
            start_element("staffNum", k_no_imoid, k_in_same_line);
            m_source << (m_pObj->get_staff() + 1);
            end_element(k_in_same_line);
#endif
        }
    }

    void add_relobjs()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (!(pRO->is_chord() || pRO->is_tie() || pRO->is_slur()
                      || pRO->is_beam() || pRO->is_tuplet()) )
                {
                    //AWARE: chords, ties, slurs are specific for notes and
                    //are generated in NoteLdpGenerator
                    //AWARE: beams and tuplets are specific for notes and rests, and
                    //are generated in source_for_noterest_options()
                    source_for_relobj(pRO, m_pObj);
                }
            }
        }
    }

};

//---------------------------------------------------------------------------------------
//@ <staffObjOptions> = { <staffNum> | <printOptions> }
class StaffObjOptionsLdpGenerator : public LdpGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjOptionsLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        add_staff_num();
        source_for_print_options(m_pObj);
        return m_source.str();
    }

protected:

    void add_staff_num()
    {
        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
            && !m_pObj->is_time_signature()
            && !m_pObj->is_barline()
            && !m_pObj->is_go_back_fwd()
            && !m_pObj->is_note_rest() )
        {
            m_source << " p" << (m_pObj->get_staff() + 1);
            space_needed();
        }
    }

};

//---------------------------------------------------------------------------------------
class SystemBreakLdpGenerator : public LdpGenerator
{
protected:
    ImoSystemBreak* m_pImo;

public:
    SystemBreakLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pImo = static_cast<ImoSystemBreak*>( pImo );
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("newSystem", m_pImo->get_id());
        end_element(k_in_same_line);
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class TieLdpGenerator : public LdpGenerator
{
protected:
    ImoTie* m_pTie;
    ImoNote* m_pNote;

public:
    TieLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pTie( static_cast<ImoTie*>(pImo) )
        , m_pNote(nullptr)
    {
    }

    string generate_source(ImoObj* pParent=nullptr)
    {
        m_pNote = static_cast<ImoNote*>( pParent );

        start_element("tie", m_pTie->get_id());
        add_tie_number();
        bool fStart = (m_pNote == m_pTie->get_start_note());
        add_tie_type(fStart);
        add_bezier_info(fStart);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_tie_number()
    {
        m_source << " " << m_pTie->get_tie_number();
    }

    void add_tie_type(bool fStart)
    {
        m_source << (fStart ? " start" : " stop");
    }

    void add_bezier_info(bool fStart)
    {
        ImoBezierInfo* pInfo = (fStart ? m_pTie->get_start_bezier()
                                       : m_pTie->get_stop_bezier() );
        if (pInfo)
        {
            static string sPointNames[4] = { "start", "end", "ctrol1", "ctrol2" };

            bool fElementStarted = false;
            for (int i=0; i < 4; i++)
            {
                TPoint& pt = pInfo->get_point(i);

                if (pt.x != 0.0f || pt.y != 0.0f)
                {
                    if (!fElementStarted)
                    {
                        start_element("bezier", k_no_imoid);
                        fElementStarted = true;
                    }

                    if (pt.x != 0.0f)
                    {
                        start_element( sPointNames[i] + "-x", k_no_imoid, k_in_same_line);
                        m_source << pt.x;
                        end_element(k_in_same_line);
                    }
                    if (pt.y != 0.0f)
                    {
                        start_element( sPointNames[i] + "-y", k_no_imoid, k_in_same_line);
                        m_source << pt.y;
                        end_element(k_in_same_line);
                    }
                }
            }
            if (fElementStarted)
                end_element();
        }
    }

};

//---------------------------------------------------------------------------------------
//@ <timeSignature> = (time [<type>] { (<top><bottom>)+ | "senza-misura" }
//@                    <staffobjOptions>* <attachments>*)
class TimeSignatureLdpGenerator : public LdpGenerator
{
protected:
    ImoTimeSignature* m_pObj;

public:
    TimeSignatureLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoTimeSignature*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("time", m_pObj->get_id());
        add_content();
        source_for_staffobj_options(m_pObj);
        source_for_attachments(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_content()
    {
        if (m_pObj->is_normal())
            m_source << " " << m_pObj->get_top_number() << " "
                     << m_pObj->get_bottom_number();
        else if (m_pObj->is_common())
            m_source << " common";
        else if (m_pObj->is_cut())
            m_source << " cut";
        else if (m_pObj->is_single_number())
            m_source << " single-number " << m_pObj->get_top_number();

        space_needed();
    }

};

//---------------------------------------------------------------------------------------
class TitleLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreTitle* m_pObj;

public:
    TitleLdpGenerator(ImoObj* pImo, LdpExporter* pExporter, bool fSpaceNeeded)
        : LdpGenerator(pExporter, fSpaceNeeded)
    {
        m_pObj = static_cast<ImoScoreTitle*>(pImo);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        start_element("title", m_pObj->get_id());
        add_text();
        add_style( m_pObj->get_style() );
        add_location_if_not_zero(m_pObj->get_user_location_x(),
                                 m_pObj->get_user_location_y());
        end_element();
        return m_source.str();
    }

protected:

    void add_text()
    {
        m_source << "\"" << m_pObj->get_text() << "\"";
    }

};

//---------------------------------------------------------------------------------------
class TupletLdpGenerator : public LdpGenerator
{
protected:
    ImoTuplet* m_pTuplet;
    ImoNoteRest* m_pNR;

public:
    TupletLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pTuplet( static_cast<ImoTuplet*>(pImo) )
        , m_pNR(nullptr)
    {
    }

    string generate_source(ImoObj* pParent=nullptr)
    {
        m_pNR = static_cast<ImoNoteRest*>( pParent );

        if (m_pNR == m_pTuplet->get_start_object())
        {
            start_element("t", m_pTuplet->get_id(), k_in_same_line);
            add_tuplet_number();
            add_tuplet_type(true);
            add_actual_notes();
            add_normal_notes();
            add_tuplet_options();
            end_element(k_in_same_line);
        }
        else if (m_pNR == m_pTuplet->get_end_object())
        {
            start_element("t", m_pTuplet->get_id(), k_in_same_line);
            add_tuplet_number();
            add_tuplet_type(false);
            end_element(k_in_same_line);
        }
        else
        {
            m_source.clear();
        }
        return m_source.str();
    }

protected:

    inline void add_tuplet_number()
    {
        m_source << " " << m_pTuplet->get_id();
        space_needed();
    }

    inline void add_tuplet_type(bool fStart)
    {
        m_source << (fStart ? " +" : " -");
    }

    inline void add_actual_notes()
    {
        m_source << " " << m_pTuplet->get_actual_number();
    }

    inline void add_normal_notes()
    {
        m_source << " " << m_pTuplet->get_normal_number();
    }

    inline void add_tuplet_options()
    {
        add_bracket_type();
        add_display_bracket();
        add_display_number();
    }

    void add_bracket_type()
    {
    }

    void add_display_bracket()
    {
        int opt = m_pTuplet->get_show_bracket();
        if (opt == k_yesno_no)
        {
            m_source << " noBracket";
            space_needed();
        }
    }

    void add_display_number()
    {
        int number = m_pTuplet->get_show_number();
        if (number == ImoTuplet::k_number_actual)
            return;     //default option

        start_element("displayNumber", k_no_imoid, k_in_same_line);

        if (number == ImoTuplet::k_number_both)
            m_source << " both";
        else if (number == ImoTuplet::k_number_none)
            m_source << " none";
        else
        {
            m_source << " both";
            stringstream s;
            s << "Invalid option. Value=" << number;
            LOMSE_LOG_ERROR(s.str());
        }

        end_element(k_in_same_line);
    }

};

//---------------------------------------------------------------------------------------
//AWARE: Must be defined after TitleLdpGenerator and DefineStyleLdpGenerator as uses both

class ScoreLdpGenerator : public LdpGenerator
{
protected:
    ImoScore* m_pObj;

public:
    ScoreLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
        pExporter->set_current_score(m_pObj);
    }

    string generate_source(ImoObj* UNUSED(pParent) =nullptr)
    {
        //TODO: commented elements

        start_element("score", m_pObj->get_id());
        add_version();
        add_style( m_pObj->get_style() );
        //add_undo_data();
        //add_creation_mode();
        add_styles();
        add_titles();
        //add_page_layout();
        add_system_layout();
        add_options();
        add_parts();
        add_instruments();
        end_element();
        return m_source.str();
    }

protected:

    void add_version()
    {
        m_source << " (vers 2.0)";
    }

    void add_undo_data()
    {
    ////ID counter value for undo/redo
    //if (fUndoData)
    //    sSource += wxString::Format(_T("   (undoData (idCounter  %d))\n"), m_nCounterID);
    }

    void add_creation_mode()
    {
//    //creation mode
//    if (!m_sCreationModeName.empty())
//    {
//        m_source << _T("   (creationMode ");
//        m_source << m_sCreationModeName;
//        m_source << _T(" ");
//        m_source << m_sCreationModeVers;
//        m_source << _T(")\n");
//    }
    }

    void add_styles()
    {
        map<std::string, ImoStyle*>& styles = m_pObj->get_styles();
        map<std::string, ImoStyle*>::const_iterator it;
        for (it = styles.begin(); it != styles.end(); ++it)
        {
            if (! (it->second)->is_default_style_with_default_values() )
//            if (!(it->first == "Default style"
//                  || it->first == "Instrument names"
//                  || it->first == "Tuplet numbers"
//                ))
            {
                DefineStyleLdpGenerator gen(it->second, m_pExporter, is_space_needed());
                m_source << gen.generate_source();
            }
        }
    }

    void add_titles()
    {
        list<ImoScoreTitle*>& titles = m_pObj->get_titles();
        list<ImoScoreTitle*>::iterator it;
        for (it = titles.begin(); it != titles.end(); ++it)
        {
            TitleLdpGenerator gen(*it, m_pExporter, is_space_needed());
            m_source << gen.generate_source();
        }
    }

    void add_page_layout()
    {
//    //first section layout info
//    //TODO: sections
//    //int nSection = 0;
//    {
//        //page layout info
//#if 0
//		std::list<lmPageInfo*>::iterator it;
//		for (it = m_PagesInfo.begin(); it != m_PagesInfo.end(); ++it)
//            m_source << (*it)->SourceLDP(1, fUndoData);
//#else
//        lmPageInfo* pPageInfo = m_PagesInfo.front();
//        m_source << pPageInfo->SourceLDP(1, fUndoData);
//#endif
//        //first system and other systems layout info
//        m_source << m_SystemsInfo.front()->SourceLDP(1, true, fUndoData);
//        m_source << m_SystemsInfo.back()->SourceLDP(1, false, fUndoData);
//    }
    }

    void add_system_layout()
    {
        ImoSystemInfo* pInfo = m_pObj->get_first_system_info();
        if (!m_pObj->has_default_values(pInfo))
            add_system_info(pInfo);

        pInfo = m_pObj->get_other_system_info();
        if (!m_pObj->has_default_values(pInfo))
            add_system_info(pInfo);
    }

    void add_system_info(ImoSystemInfo* pInfo)
    {
        start_element("systemLayout", pInfo->get_id(), k_in_new_line);
        m_source << (pInfo->is_first() ? " first" : " other") << " ";
        start_element("systemMargins", k_no_imoid);
        m_source << " " << pInfo->get_left_margin() << " "
                 << pInfo->get_right_margin() << " "
                 << pInfo->get_system_distance() << " "
                 << pInfo->get_top_system_distance();
        end_element(k_in_same_line);
        end_element(k_in_same_line);
    }

    void add_options()
    {
        ImoOptions* pColOpts = m_pObj->get_options();
        ImoObj::children_iterator it;
        for (it= pColOpts->begin(); it != pColOpts->end(); ++it)
        {
            ImoOptionInfo* pOpt = static_cast<ImoOptionInfo*>(*it);
            if (!m_pObj->has_default_value(pOpt))
            {
                start_element("opt", pOpt->get_id(), k_in_new_line);
                m_source << " " << pOpt->get_name() << " ";
                if (pOpt->is_bool_option())
                    m_source << (pOpt->get_bool_value() ? "true" : "false");
                else if (pOpt->is_long_option())
                    m_source << pOpt->get_long_value();
                else if (pOpt->is_float_option())
                    m_source << pOpt->get_float_value();
                else
                    m_source << pOpt->get_string_value();
                end_element(k_in_same_line);
            }
        }
    }

    void add_parts()
    {
        ImoInstrGroups* pGroups = m_pObj->get_instrument_groups();
        if (pGroups == nullptr)
            return;

        start_element("parts", k_no_imoid, k_in_new_line);
        add_instr_ids();
        add_groups();
        end_element(k_in_new_line);
    }

    void add_instr_ids()
    {
        start_element("instrIds", k_no_imoid, k_in_new_line);
        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
        {
            ImoInstrument* pInstr = m_pObj->get_instrument(i);
            if (i > 0)
                m_source << " ";
            m_source << pInstr->get_instr_id();
        }
        end_element(k_in_same_line);
    }

    void add_groups()
    {
        ImoInstrGroups* pGroups = m_pObj->get_instrument_groups();
        ImoObj::children_iterator it;
        for (it= pGroups->begin(); it != pGroups->end(); ++it)
        {
            start_element("group", k_no_imoid, k_in_new_line);

            ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(*it);
            ImoInstrument* pInstr = pGrp->get_first_instrument();
            m_source << pInstr->get_instr_id();
            pInstr = pGrp->get_last_instrument();
            m_source << " " << pInstr->get_instr_id();
            bool fAddSpace = true;

            string value = pGrp->get_name_string();
            if (!value.empty())
            {
                if (fAddSpace)
                {
                    m_source << " ";
                    fAddSpace = false;
                }
                start_element("name", k_no_imoid, k_in_same_line);
                m_source << "\"" << value << "\"";
                end_element(k_in_same_line);
            }


            value = pGrp->get_abbrev_string();
            if (!value.empty())
            {
                if (fAddSpace)
                {
                    m_source << " ";
                    fAddSpace = false;
                }
                start_element("abbrev", k_no_imoid, k_in_same_line);
                m_source << "\"" << value << "\"";
                end_element(k_in_same_line);
            }

            if (pGrp->get_symbol() != ImoInstrGroup::k_none)
            {
                if (fAddSpace)
                {
                    m_source << " ";
                    fAddSpace = false;
                }
                start_element("symbol", k_no_imoid, k_in_same_line);
                switch (pGrp->get_symbol())
                {
                    case ImoInstrGroup::k_bracket:
                        m_source << "bracket";
                        break;
                    case ImoInstrGroup::k_brace:
                        m_source << "brace";
                        break;
                    case ImoInstrGroup::k_line:
                        m_source << "line";
                        break;
                    default:
                        m_source << "none";
                }
                end_element(k_in_same_line);
            }

            if (pGrp->join_barlines() != ImoInstrGroup::k_no)
            {
                if (fAddSpace)
                {
                    m_source << " ";
                    fAddSpace = false;
                }
                start_element("joinBarlines", k_no_imoid, k_in_same_line);
                switch (pGrp->join_barlines())
                {
                    case ImoInstrGroup::k_standard:
                        m_source << " yes";
                        break;
                    case ImoInstrGroup::k_mensurstrich:
                        m_source << " mensurstrich";
                        break;
                    default:
                        m_source << " no";
                }
                end_element(k_in_same_line);
            }

            end_element(k_in_same_line);    //group
        }
    }

    void add_instruments()
    {
        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
            add_source_for( m_pObj->get_instrument(i) );
    }


};


//=======================================================================================
// LdpGenerator implementation
//=======================================================================================
void LdpGenerator::start_element(const string& name, ImoId id, bool fInNewLine)
{
    if (fInNewLine)
        new_line_and_indent_spaces();

    if (m_fAddSpace)
        m_source << " ";
    m_source << "(" << name;
    if (m_pExporter->get_add_id() && id != k_no_imoid)
        m_source << "#" << std::dec << id;
    increment_indent();
    m_fAddSpace = false;
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_element(bool fStartLine)
{
    decrement_indent();
    if (fStartLine)
        new_line_and_indent_spaces(fStartLine);
    m_source << ")";
    m_fAddSpace = false;
}

//---------------------------------------------------------------------------------------
void LdpGenerator::start_comment()
{
    new_line_and_indent_spaces();
    m_source << "/* ";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_comment()
{
    m_source << " */";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::empty_line()
{
    m_source.clear();
    new_line();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::new_line_and_indent_spaces(bool fStartLine)
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
void LdpGenerator::new_line()
{
    if (!m_pExporter->get_remove_newlines())
        m_source << endl;
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_source_for(ImoObj* pImo)
{
    add_space_if_needed();
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_abbreviated_elements(ImoNoteRest* pNR)
{
    //voice
    m_source << " v" << pNR->get_voice();

    //staffNum
    m_source << " p" << (pNR->get_staff() + 1);
    space_needed();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_noterest_options(ImoNoteRest* pNR)
{
    //@ <noteRestOptions> = { <beam> | <tuplet> }

    if (pNR->get_num_relations() > 0)
    {
        ImoRelations* pRelObjs = pNR->get_relations();
        int size = pRelObjs->get_num_items();
        for (int i=0; i < size; ++i)
        {
            ImoRelObj* pRO = pRelObjs->get_item(i);
            if (pRO->is_tuplet() )
            {
                TupletLdpGenerator gen(pRO, m_pExporter);
                string src = gen.generate_source(pNR);
                if (!src.empty())
                {
                    add_space_if_needed();
                    m_source << src;
                }
            }

            else if (pRO->is_beam() )
            {
                BeamLdpGenerator gen(pRO, m_pExporter);
                string src = gen.generate_source(pNR);
                if (!src.empty())
                {
                    add_space_if_needed();
                    m_source << src;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_staffobj_options(ImoStaffObj* pSO)
{
//@ <staffObjOptions> = { <staffNum> | <printOptions> }

    StaffObjOptionsLdpGenerator gen(pSO, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_print_options(ImoScoreObj* pSO)
{
//@ <printOptions> = { [<visible>] [<location>] [<color>] }

    PrintOptionsLdpGenerator gen(pSO, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_attachments(ImoContentObj* pSO)
{
    if (pSO->get_num_attachments() > 0)
    {
        ImoAttachments* pAuxObjs = pSO->get_attachments();
        int size = pAuxObjs->get_num_items();
        for (int i=0; i < size; ++i)
        {
            ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
            if (!(pSO->is_note() && pAO->is_lyric()) )
                source_for_auxobj(pAO);
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjLdpGenerator gen(pImo, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjLdpGenerator gen(pImo, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjLdpGenerator gen(pImo, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_imobj(ImoObj* pImo)
{
    increment_indent();
    ImObjLdpGenerator gen(pImo, m_pExporter, is_space_needed());
    m_source << gen.generate_source();
    decrement_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_auxobj(ImoObj* pImo)
{
    if (!pImo->is_lyric())
    {
        //AWARE: Lyrics are generated in note generator
        string src = m_pExporter->get_source(pImo);
        if (!src.empty())
        {
            add_space_if_needed();
            m_source << src;
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_relobj(ImoObj* pRO, ImoObj* pParent)
{
    string src = m_pExporter->get_source(pRO, pParent);
    if (!src.empty())
    {
        add_space_if_needed();
        m_source << src;
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::increment_indent()
{
    m_pExporter->increment_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::decrement_indent()
{
    m_pExporter->decrement_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_duration(stringstream& source, int noteType, int dots)
{
    source << " " << LdpExporter::notetype_to_string(noteType, dots);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_visible(bool fVisible)
{
    if (!fVisible)
    {
        add_space_if_needed();
        m_source << "(visible no)";
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_color_if_not_black(Color color)
{
    if (color.r != 0 || color.g != 0  || color.b != 0 || color.a != 255)
    {
        add_space_if_needed();
        m_source << "(color " << LdpExporter::color_to_ldp(color) << ")";
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_width_if_not_default(Tenths width, Tenths def)
{
    if (width != def)
    {
        start_element("width", k_no_imoid, k_in_same_line);
        m_source << " " << width;
        end_element(k_in_same_line);
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_location_if_not_zero(Tenths x, Tenths y)
{
    if (x != 0.0f || y != 0.0f)
    {
        if (x != 0.0f)
        {
            start_element("dx", k_no_imoid, k_in_same_line);
            m_source << " " << x;
            end_element(k_in_same_line);
        }
        if (y != 0.0f)
        {
            start_element("dy", k_no_imoid, k_in_same_line);
            m_source << " " << y;
            end_element(k_in_same_line);
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_location(TPoint pt)
{
    start_element("dx", k_no_imoid, k_in_same_line);
    m_source << " " << pt.x;
    end_element(k_in_same_line);

    start_element("dy", k_no_imoid, k_in_same_line);
    m_source << " " << pt.y;
    end_element(k_in_same_line);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_placement(int placement)
{
    if (placement == k_placement_default)
        return;
    else if (placement == k_placement_above)
        m_source << " above";
    else
        m_source << " below";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_style(ImoStyle* pStyle)
{
    if (pStyle && pStyle->get_name() != "Default style")
    {
        start_element("style", k_no_imoid);
        m_source << " \"" << pStyle->get_name() << "\"";
        end_element(k_in_same_line);
    }
}


//=======================================================================================
// LdpExporter implementation
//=======================================================================================
LdpExporter::LdpExporter(LibraryScope* pLibraryScope)
    : m_pLibraryScope(pLibraryScope)
    , m_version( pLibraryScope->get_version_string() )
    , m_nIndent(0)
    , m_fAddId(false)
    , m_fRemoveNewlines(false)
    , m_pCurrScore(nullptr)
    , m_fProcessingChord(false)
{
}

LdpExporter::LdpExporter()
    : m_pLibraryScope(nullptr)
    , m_version()
    , m_nIndent(0)
    , m_fAddId(false)
    , m_fRemoveNewlines(false)
    , m_pCurrScore(nullptr)
    , m_fProcessingChord(false)
{
}

//---------------------------------------------------------------------------------------
LdpExporter::~LdpExporter()
{
}

//---------------------------------------------------------------------------------------
string LdpExporter::get_source(ImoObj* pImo, ImoObj* pParent)
{
    LdpGenerator* pGen = new_generator(pImo);
    string source = pGen->generate_source(pParent);
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
LdpGenerator* LdpExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_articulation_symbol:
                                    return LOMSE_NEW ArticulationSymbolLdpGenerator(pImo, this);
        case k_imo_barline:         return LOMSE_NEW BarlineLdpGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefLdpGenerator(pImo, this);
        case k_imo_direction:       return LOMSE_NEW DirectionLdpGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocLdpGenerator(pImo, this);
        case k_imo_dynamics_mark:   return LOMSE_NEW DynamicsLdpGenerator(pImo, this);
        case k_imo_fermata:         return LOMSE_NEW FermataLdpGenerator(pImo, this);
//        case k_imo_figured_bass:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_go_back_fwd:     return LOMSE_NEW GoBackFwdLdpGenerator(pImo, this);
        //AWARE: goBack is needed for exporting 1.6 to 2.0
        case k_imo_instrument:      return LOMSE_NEW InstrumentLdpGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureLdpGenerator(pImo, this);
        case k_imo_lyric:           return LOMSE_NEW LyricLdpGenerator(pImo, this);
        case k_imo_metronome_mark:  return LOMSE_NEW MetronomeLdpGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataLdpGenerator(pImo, this);
        case k_imo_note:            return LOMSE_NEW NoteLdpGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW RestLdpGenerator(pImo, this);
        case k_imo_system_break:    return LOMSE_NEW SystemBreakLdpGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreLdpGenerator(pImo, this);
        case k_imo_score_text:      return LOMSE_NEW ScoreTextLdpGenerator(pImo, this);
        case k_imo_score_line:      return LOMSE_NEW ScoreLineLdpGenerator(pImo, this);
        case k_imo_slur:            return LOMSE_NEW SlurLdpGenerator(pImo, this);
        case k_imo_time_signature:  return LOMSE_NEW TimeSignatureLdpGenerator(pImo, this);
        case k_imo_tie:             return LOMSE_NEW TieLdpGenerator(pImo, this);
        default:
            return new ErrorLdpGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
string LdpExporter::clef_type_to_ldp(int clefType)
{
    //AWARE: indexes in correspondence with enum k_clef__type
    static const string name[] = {
        "G",
        "F4",
        "F3",
        "C1",
        "C2",
        "C3",
        "C4",
        "percussion",
        "C5",
        "F5",
        "G1",
        "8_G",     //8 above
        "G_8",     //8 below
        "8_F4",    //8 above
        "F4_8",    //8 below
        "15_G2",   //15 above
        "G2_15",   //15 below
        "15_F4",   //15 above
        "F4_15",   //15 below
    };
    static const string undefined = "undefined";


    if (clefType == k_clef_undefined)
        return undefined;
    else
        return name[clefType];
}
//---------------------------------------------------------------------------------------
string LdpExporter::key_type_to_ldp(int keyType)
{
    switch(keyType)
    {
        case k_key_C:   return "C";
        case k_key_G:   return "G";
        case k_key_D:   return "D";
        case k_key_A:   return "A";
        case k_key_E:   return "E";
        case k_key_B:   return "B";
        case k_key_Fs:  return "F+";
        case k_key_Cs:  return "C+";
        case k_key_Cf:  return "C-";
        case k_key_Gf:  return "G-";
        case k_key_Df:  return "D-";
        case k_key_Af:  return "A-";
        case k_key_Ef:  return "E-";
        case k_key_Bf:  return "B-";
        case k_key_F:   return "F";
        case k_key_a:   return "a";
        case k_key_e:   return "e";
        case k_key_b:   return "b";
        case k_key_fs:  return "f+";
        case k_key_cs:  return "c+";
        case k_key_gs:  return "g+";
        case k_key_ds:  return "d+";
        case k_key_as:  return "a+";
        case k_key_af:  return "a-";
        case k_key_ef:  return "e-";
        case k_key_bf:  return "b-";
        case k_key_f:   return "f";
        case k_key_c:   return "c";
        case k_key_g:   return "g";
        case k_key_d:   return "d";
        default:
            return "undefined";
    }
}

//---------------------------------------------------------------------------------------
string LdpExporter::barline_type_to_ldp(int barlineType)
{
    switch(barlineType)
    {
        case k_barline_end_repetition:
            return "endRepetition";
        case k_barline_start_repetition:
            return "startRepetition";
        case k_barline_end:
            return "end";
        case k_barline_double:
            return "double";
        case k_barline_simple:
            return "simple";
        case k_barline_start:
            return "start";
        case k_barline_double_repetition:
        case k_barline_double_repetition_alt:
            return "doubleRepetition";
        case k_barline_none:
            return "none";
        default:
            return "undefined";
    }
}

//---------------------------------------------------------------------------------------
string LdpExporter::color_to_ldp(Color color)
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
string LdpExporter::float_to_string(float num)
{
    stringstream source;
    source << num;
    return source.str();
}

//---------------------------------------------------------------------------------------
string LdpExporter::accidentals_to_string(int acc)
{
    stringstream source;
    switch(acc)
    {
        case k_invalid_accidentals:     break;
        case k_no_accidentals:          break;
        case k_sharp:                   source << "+";  break;
        case k_sharp_sharp:             source << "++";  break;
        case k_double_sharp:            source << "x";  break;
        case k_natural_sharp:           source << "=+";  break;
        case k_flat:                    source << "-";  break;
        case k_flat_flat:               source << "--";  break;
        case k_natural_flat:            source << "=-";  break;
        case k_natural:                 source << "=";   break;
        default:                        break;
    }
    return source.str();
}

//---------------------------------------------------------------------------------------
string LdpExporter::notetype_to_string(int noteType, int dots)
{
    stringstream source;
    switch(noteType)
    {
        case k_longa:   source << "l";  break;
        case k_breve:   source << "b";  break;
        case k_whole:   source << "w";  break;
        case k_half:    source << "h";  break;
        case k_quarter: source << "q";  break;
        case k_eighth:  source << "e";  break;
        case k_16th:    source << "s";  break;
        case k_32nd:    source << "t";  break;
        case k_64th:    source << "i";  break;
        case k_128th:   source << "o";  break;
        case k_256th:   source << "f";  break;
        default:
        {
            source << noteType << "?";
            stringstream s;
            s << "Invalid noteType. Value=" << noteType;
            LOMSE_LOG_ERROR(s.str());
        }
    }

    while (dots > 0)
    {
        source << ".";
        --dots;
    }

    return source.str();
}



}  //namespace lomse
