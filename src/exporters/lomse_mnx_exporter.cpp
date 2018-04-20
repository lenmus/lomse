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

#include "lomse_mnx_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_mnx_exporter.h"
#include "lomse_logger.h"
#include "lomse_time.h"

#include <stack>
using namespace std;

namespace lomse
{

//=======================================================================================
// MnxGenerator
//=======================================================================================
class MnxGenerator
{
protected:
    MnxExporter* m_pExporter;
    stringstream m_source;

public:
    MnxGenerator(MnxExporter* pExporter);
    virtual ~MnxGenerator() {}

    virtual string generate_source() = 0;

protected:
    void start_element(const string& name, ImoObj* pImo);
    void close_start_tag();
    void start_attrib(const string& name);
    void end_attrib();
    void add_attribute(const string& name, const string& value);
    void end_element(bool fInNewLine = true, bool fNewElement=true);
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void new_line();
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

    void add_duration(stringstream& source, int noteType, int dots);
    void add_optional_style(ImoContentObj* pObj);

};

const bool k_in_same_line = false;
const bool k_in_new_line = true;
const int k_indent_step = 3;
const bool k_add_close_tag = false;
const bool k_add_close_element = true;

//=======================================================================================
// generators for specific elements
//=======================================================================================


//---------------------------------------------------------------------------------------
class xxxxxxMnxGenerator : public MnxGenerator
{
protected:
    //ImoXXXXX* m_pObj;

public:
    xxxxxxMnxGenerator(ImoObj* UNUSED(pImo), MnxExporter* pExporter)
        : MnxGenerator(pExporter)
    {
        //m_pObj = static_cast<ImoXXXXX*>(pImo);
    }

    string generate_source()
    {
        //start_element("xxxxx", m_pObj);
        close_start_tag();
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class BarlineMnxGenerator : public MnxGenerator
{
protected:
    ImoBarline* m_pObj;

public:
    BarlineMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    string generate_source()
    {
        start_element("barline", m_pObj);
        close_start_tag();
        add_barline_type();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_barline_type()
    {
        m_source << MnxExporter::barline_type_to_mnx( m_pObj->get_type() );
    }

};


//---------------------------------------------------------------------------------------
class ClefMnxGenerator : public MnxGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    string generate_source()
    {
        if (m_pExporter->current_open_tag() != "directions")
        {
            start_element("directions", nullptr);
            close_start_tag();
        }
        start_element("clef", m_pObj);
        add_line_sign();
        //source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line, k_add_close_tag);
        return m_source.str();
    }

protected:

    void add_line_sign()
    {
        string clef = MnxExporter::clef_type_to_mnx( m_pObj->get_clef_type() );
        start_attrib("line");
        m_source << clef.back();
        end_attrib();

        start_attrib("sign");
        m_source << clef.front();
        end_attrib();
    }

};


//---------------------------------------------------------------------------------------
class ContentObjMnxGenerator : public MnxGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoContentObj*>(pImo);
    }

    string generate_source()
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
            m_source << " (dx " << MnxExporter::float_to_string(ux) << ")";

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
            m_source << " (dy " << MnxExporter::float_to_string(uy) << ")";
    }

    void add_attachments()
    {
        if (m_pObj->get_num_attachments() > 0)
        {
            increment_indent();
//            std::list<ImoAuxObj*>& attachments = m_pObj->get_attachments();
//            std::list<ImoAuxObj*>::iterator it;
//            for (it = attachments.begin(); it != attachments.end(); ++it)
//            {
//                ImoAuxObj* pAuxObj = *it;
//                if ( pAuxObj->is_relobj() )
//                {
//                    ImRelObj* pRO = dynamic_cast<ImRelObj*)>(pAuxObj);
//
//                    //exclude beams, as source code for them is generted in ImoNote.
//                    //AWARE. This is necessary because MNX parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if ( pRO->GetStartNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceMNX_First(nIndent, fUndoData, (lmNoteRest*)this);
//                        else if ( pRO->GetEndNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceMNX_Last(nIndent, fUndoData, (lmNoteRest*)this);
//                        else
//                            m_source += pRO->SourceMNX_Middle(nIndent, fUndoData, (lmNoteRest*)this);
//                    }
//                }
//                else if ( pAuxObj->IsRelObX() )
//                {
//                    lmRelObX* pRO = (lmRelObX*)pAuxObj;
//
//                    //exclude beams, as source code for them is generted in ImoNote.
//                    //AWARE. This is necessary because MNX parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if (pRO->GetStartSO() == this)
//                            m_source += pRO->SourceMNX_First(nIndent, fUndoData, this);
//                        else if (pRO->GetEndSO() == this)
//                            m_source += pRO->SourceMNX_Last(nIndent, fUndoData, this);
//                        else
//                            m_source += pRO->SourceMNX_Middle(nIndent, fUndoData, this);
//                    }
//                }
//                else
//                    source_for_auxobj(pAuxObj);
//            }
            decrement_indent();
        }
    }
};


//---------------------------------------------------------------------------------------
class DefineStyleMnxGenerator : public MnxGenerator
{
protected:
    ImoStyle* m_pObj;

public:
    DefineStyleMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStyle*>(pImo);
    }

    string generate_source()
    {
        start_element("defineStyle", m_pObj);
        close_start_tag();
        add_name();
        add_properties();
        end_element();
        return m_source.str();
    }

protected:

    void add_name()
    {
        start_element("name", nullptr);
        close_start_tag();
        m_source << m_pObj->get_name();
        end_element(k_in_same_line);
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
            start_element("font-size", nullptr);
            close_start_tag();
            m_source << rValue << "pt";
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
            create_lunits_element("", uValue);

        if (m_pObj->get_lunits_property(ImoStyle::k_word_spacing_length, &uValue))
            create_lunits_element("", uValue);

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
        start_element(tag, nullptr);
        close_start_tag();
        m_source << value;
        end_element(k_in_same_line);
    }

    void create_float_element(const string& tag, float value)
    {
        start_element(tag, nullptr);
        close_start_tag();
        m_source << value;
        end_element(k_in_same_line);
    }

    void create_lunits_element(const string& tag, LUnits value)
    {
        start_element(tag, nullptr);
        close_start_tag();
        m_source << value;
        end_element(k_in_same_line);
    }

    void create_int_element(const string& tag, int value)
    {
        start_element(tag, nullptr);
        close_start_tag();
        m_source << value;
        end_element(k_in_same_line);
    }

    void create_color_element(const string& tag, Color color)
    {
        start_element(tag, nullptr);
        close_start_tag();

        m_source << MnxExporter::color_to_mnx(color);

        end_element(k_in_same_line);
    }

    void create_font_style(int value)
    {
        start_element("font-style", nullptr);
        close_start_tag();

        if (value == ImoStyle::k_font_style_normal)
            m_source << "normal";
        else if (value == ImoStyle::k_font_style_italic)
            m_source << "italic";
        else
            m_source << "invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_font_weight(int value)
    {
        start_element("font-weight", nullptr);
        close_start_tag();

        if (value == ImoStyle::k_font_weight_normal)
            m_source << "normal";
        else if (value == ImoStyle::k_font_weight_bold)
            m_source << "bold";
        else
            m_source << "invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_text_decoration(int value)
    {
        start_element("text-decoration", nullptr);
        close_start_tag();

        if (value == ImoStyle::k_decoration_none)
            m_source << "none";
        else if (value == ImoStyle::k_decoration_underline)
            m_source << "underline";
        else if (value == ImoStyle::k_decoration_overline)
            m_source << "overline";
        else if (value == ImoStyle::k_decoration_line_through)
            m_source << "line-through";
        else
            m_source << "invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_vertical_align(int value)
    {
        start_element("vertical-align", nullptr);
        close_start_tag();

        if (value == ImoStyle::k_valign_baseline)
            m_source << "baseline";
        else if (value == ImoStyle::k_valign_sub)
            m_source << "sub";
        else if (value == ImoStyle::k_valign_super)
            m_source << "super";
        else if (value == ImoStyle::k_valign_top)
            m_source << "top";
        else if (value == ImoStyle::k_valign_text_top)
            m_source << "text-top";
        else if (value == ImoStyle::k_valign_middle)
            m_source << "middle";
        else if (value == ImoStyle::k_valign_bottom)
            m_source << "bottom";
        else if (value == ImoStyle::k_valign_text_bottom)
            m_source << "text-bottom";
        else
            m_source << "invalid value " << value;

        end_element(k_in_same_line);
    }

    void create_text_align(int value)
    {
        start_element("text-align", nullptr);
        close_start_tag();

        if (value == ImoStyle::k_align_left)
            m_source << "left";
        else if (value == ImoStyle::k_align_right)
            m_source << "right";
        else if (value == ImoStyle::k_align_center)
            m_source << "center";
        else if (value == ImoStyle::k_align_justify)
            m_source << "justify";
        else
            m_source << "invalid value " << value;

        end_element(k_in_same_line);
    }

};


//---------------------------------------------------------------------------------------
class ErrorMnxGenerator : public MnxGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorMnxGenerator(ImoObj* pImo, MnxExporter* pExporter)
        : MnxGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source()
    {
        start_element("TODO", m_pImo);
        close_start_tag();
        m_source << "Error: no MnxExporter for Imo. Imo name=" << m_pImo->get_name()
                 << ", Imo type=" << m_pImo->get_obj_type()
                 << ", id=" << m_pImo->get_id();
        end_element(k_in_same_line);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ImObjMnxGenerator : public MnxGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = pImo;
    }

    string generate_source()
    {
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class InstrumentMnxGenerator : public MnxGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoInstrument*>(pImo);
    }

    string generate_source()
    {
        start_element("part", m_pObj);
        close_start_tag();
        add_num_staves();
        add_sound_info();
        add_name_abbreviation();
        add_music_data();
        end_element();
        empty_line();
        return m_source.str();
    }

protected:

    void add_num_staves()
    {
	    //sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(staves %d)\n"), m_pVStaff->GetNumStaves());
     //   int nStaves = m_pVStaff->GetNumStaves();
     //   for (int i=0; i < nStaves; i++)
     //       sSource += m_pVStaff->GetStaff(i+1)->SourceMNX(nIndent, fUndoData);
    }

    void add_sound_info()
    {
	    //sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(infoMIDI %d %d)\n"), m_nMidiInstr, m_nMidiChannel);
    }

    void add_name_abbreviation()
    {
     //   if (m_pName)
     //   {
	    //    sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
     //       sSource += m_pName->SourceMNX(_T("name"), fUndoData);
     //       sSource += _T("\n");
     //   }
     //   if (m_pAbbreviation)
     //   {
	    //    sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
     //       sSource += m_pAbbreviation->SourceMNX(_T("abbrev"), fUndoData);
     //       sSource += _T("\n");
     //   }
    }

    void add_music_data()
    {
        add_source_for( m_pObj->get_musicdata() );
    }
};


//---------------------------------------------------------------------------------------
class KeySignatureMnxGenerator : public MnxGenerator
{
protected:
    ImoKeySignature* m_pObj;

public:
    KeySignatureMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoKeySignature*>(pImo);
    }

    string generate_source()
    {
        start_element("key", m_pObj);
        close_start_tag();
        add_key_type();

        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_key_type()
    {
        switch(m_pObj->get_key_type())
        {
            case k_key_C:   m_source << "C";    break;
            case k_key_G:   m_source << "G";    break;
            case k_key_D:   m_source << "D";    break;
            case k_key_A:   m_source << "A";    break;
            case k_key_E:   m_source << "E";    break;
            case k_key_B:   m_source << "B";    break;
            case k_key_Fs:  m_source << "Fs";   break;
            case k_key_Cs:  m_source << "Cs";   break;
            case k_key_Cf:  m_source << "Cf";   break;
            case k_key_Gf:  m_source << "Gf";   break;
            case k_key_Df:  m_source << "Df";   break;
            case k_key_Af:  m_source << "Af";   break;
            case k_key_Ef:  m_source << "Ef";   break;
            case k_key_Bf:  m_source << "Bf";   break;
            case k_key_F:   m_source << "F";    break;
            case k_key_a:   m_source << "a";    break;
            case k_key_e:   m_source << "e";    break;
            case k_key_b:   m_source << "b";    break;
            case k_key_fs:  m_source << "fs";   break;
            case k_key_cs:  m_source << "cs";   break;
            case k_key_gs:  m_source << "gs";   break;
            case k_key_ds:  m_source << "ds";   break;
            case k_key_as:  m_source << "as";   break;
            case k_key_af:  m_source << "af";   break;
            case k_key_ef:  m_source << "ef";   break;
            case k_key_bf:  m_source << "bf";   break;
            case k_key_f:   m_source << "f";    break;
            case k_key_c:   m_source << "c";    break;
            case k_key_g:   m_source << "g";    break;
            case k_key_d:   m_source << "d";    break;
            default:                            break;
        }
    }

};


//---------------------------------------------------------------------------------------
class LenmusdocMnxGenerator : public MnxGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source()
    {
        m_source << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        add_comment();
        start_element("mnx", m_pObj);
        //add_attribute("vers", m_pObj->get_version());
        //add_attribute("language", m_pObj->get_language());
        close_start_tag();


        add_head();
        add_content();

        end_element();    //mnx
        return m_source.str();
    }

protected:

    void add_head()
    {
        start_element("head", m_pObj);
        close_start_tag();

        add_styles();

        end_element();
    }

    void add_styles()
    {
        ImoObj* pStyles = m_pObj->get_child_of_type(k_imo_styles);
        if (pStyles)
            add_source_for(pStyles);
    }

    void add_comment()
    {
        if (!m_pExporter->get_remove_newlines())
        {
            start_comment();
            m_source << "MNX file generated by Lomse, version "
                     << m_pExporter->get_version_and_time_string();
            end_comment();
        }
    }

    void add_content()
    {
        int numItems = m_pObj->get_num_content_items();
        for (int i=0; i < numItems; i++)
        {
            m_source << " ";
            add_source_for( m_pObj->get_content_item(i) );
        }
    }

};


//---------------------------------------------------------------------------------------
class MusicDataMnxGenerator : public MnxGenerator
{
protected:
    ImoMusicData* m_pObj;

public:
    MusicDataMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
    }

    string generate_source()
    {
        add_staffobjs();
        empty_line();
        return m_source.str();
    }

protected:

    void add_staffobjs()
    {
        ImoObj::children_iterator it = m_pObj->begin();
        bool fNewMeasure = true;
        int nMeasure = 1;
        while (it != m_pObj->end())
        {
            if (fNewMeasure)
            {
                if (!m_pExporter->get_remove_newlines())
                {
                    empty_line();

                    start_element("measure", nullptr);
                    start_attrib("number");
                    m_source << nMeasure++;
                    end_attrib();
                    close_start_tag();  //measure
                }
                fNewMeasure = false;
            }

            add_source_for(*it);

            fNewMeasure = (*it)->is_barline();
            if (fNewMeasure)
            {
                if (m_pExporter->current_open_tag() == "directions")
                    end_element();

                if (m_pExporter->current_open_tag() == "sequence")
                    end_element();

                end_element();  //measure
            }

            ++it;
        }
      //  //iterate over the collection of StaffObjs, ordered by voice.
      //  //Measures must be processed one by one
      //  for (int nMeasure=1; nMeasure <= m_cStaffObjs.GetNumMeasures(); nMeasure++)
      //  {
      //      //add comment to separate measures
      //      sSource += _T("\n");
      //      sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
      //      sSource += wxString::Format(_T("//Measure %d\n"), nMeasure);

      //      int nNumVoices = m_cStaffObjs.GetNumVoicesInMeasure(nMeasure);
      //      int nVoice = 1;
      //      while (!m_cStaffObjs.IsVoiceUsedInMeasure(nVoice, nMeasure) &&
      //          nVoice <= lmMAX_VOICE)
      //      {
      //          nVoice++;
      //      }
      //      int nVoicesProcessed = 1;   //voice 0 is automatically processed
		    //lmBarline* pBL = (lmBarline*)nullptr;
      //      bool fGoBack = false;
		    //TimeUnits rTime = 0.0;
      //      while (true)
      //      {
      //          lmSOIterator* pIT = m_cStaffObjs.CreateIterator();
      //          pIT->AdvanceToMeasure(nMeasure);
      //          while(!pIT->ChangeOfMeasure() && !pIT->EndOfCollection())
      //          {
      //              lmStaffObj* pSO = pIT->GetCurrent();
      //              //voice 0 staffobjs go with first voice if more than one voice
				  //  if (!pSO->IsBarline())
				  //  {
					 //   if (nVoicesProcessed == 1)
					 //   {
						//    if (!pSO->IsNoteRest() || ((lmNoteRest*)pSO)->GetVoice() == nVoice)
						//    {
						//	    MNX_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceMNX(nIndent, fUndoData);
						//	    rTime = MNX_AdvanceTimeCounter(pSO);
						//    }
					 //   }
					 //   else
						//    if (pSO->IsNoteRest() && ((lmNoteRest*)pSO)->GetVoice() == nVoice)
						//    {
						//	    MNX_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceMNX(nIndent, fUndoData);
						//	    rTime = MNX_AdvanceTimeCounter(pSO);
						//    }
				  //  }
				  //  else
					 //   pBL = (lmBarline*)pSO;

      //              pIT->MoveNext();
      //          }
      //          delete pIT;

      //          //check if more voices
      //          if (++nVoicesProcessed >= nNumVoices) break;
      //          nVoice++;
      //          while (!m_cStaffObjs.IsVoiceUsedInMeasure(nVoice, nMeasure) &&
      //              nVoice <= lmMAX_VOICE)
      //          {
      //              nVoice++;
      //          }
      //          wxASSERT(nVoice <= lmMAX_VOICE);

      //          //there are more voices. Add (goBak) tag
      //          fGoBack = true;
      //          sSource += _T("\n");
      //          sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
      //          sSource += _T("(goBack start)\n");
      //          rTime = 0.0;
      //      }

      //      //if goBack added, add a goFwd to ensure that we are at end of measure
      //      if (fGoBack)
      //      {
      //          sSource.append(nIndent * lmMNX_INDENT_STEP, _T(' '));
      //          sSource += _T("(goFwd end)\n");
      //      }

		    ////add barline, if present
		    //if (pBL)
			   // sSource += pBL->SourceMNX(nIndent, fUndoData);

      //  }

    }

};


//---------------------------------------------------------------------------------------
class NoteMnxGenerator : public MnxGenerator
{
protected:
    ImoNote* m_pObj;

public:
    NoteMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoNote*>(pImo);
    }

    string generate_source()
    {
        if (m_pExporter->current_open_tag() == "directions")
            end_element();

        if (m_pExporter->current_open_tag() != "sequence"
            && m_pExporter->current_open_tag() != "event")
        {
            start_element("sequence", nullptr);
            close_start_tag();
        }

        if (!m_pObj->is_in_chord() || m_pObj->is_start_of_chord())
        {
            start_element("event", nullptr);
            add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
            close_start_tag();

            m_pExporter->set_processing_chord(m_pObj->is_start_of_chord());
        }

        //export note
        start_element("note", m_pObj);
        add_pitch();
//        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line, k_add_close_tag);   //note


        if (!m_pObj->is_in_chord() || m_pObj->is_end_of_chord())
        {
            end_element();  //event
            m_pExporter->set_processing_chord(false);
        }

        return m_source.str();
    }

protected:

    void add_pitch()
    {
        static const string sNoteName[7] = { "C",  "D", "E", "F", "G", "A", "B" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pObj->get_step() == k_no_pitch)
        {
            start_attrib("pitch");
            m_source << "*";
            end_attrib();
            return;
        }

        EAccidentals acc = m_pObj->get_notated_accidentals();
        switch(acc)
        {
            case k_invalid_accidentals:     break;
            case k_no_accidentals:          break;
            case k_sharp:                   m_source << "+";  break;
            case k_sharp_sharp:             m_source << "++";  break;
            case k_double_sharp:            m_source << "x";  break;
            case k_natural_sharp:           m_source << "=+";  break;
            case k_flat:                    m_source << "-";  break;
            case k_flat_flat:               m_source << "--";  break;
            case k_natural_flat:            m_source << "=-";  break;
            case k_natural:                 m_source << "=";   break;
            default:                        break;
        }


        start_attrib("pitch");
        m_source << sNoteName[m_pObj->get_step()];
        m_source << sOctave[m_pObj->get_octave()];
        end_attrib();
    }


};


//---------------------------------------------------------------------------------------
class RestMnxGenerator : public MnxGenerator
{
protected:
    ImoRest* m_pObj;

public:
    RestMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoRest*>(pImo);
    }

    string generate_source()
    {
        if (m_pExporter->current_open_tag() == "directions")
            end_element();

        if (m_pExporter->current_open_tag() != "sequence"
            && m_pExporter->current_open_tag() != "event")
        {
            start_element("sequence", nullptr);
            close_start_tag();
        }

        start_element("event", nullptr);
        if (m_pObj->is_full_measure())
            add_attribute("measure", "yes");
        else
            add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        close_start_tag();
        start_element("rest", m_pObj);
        //source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line, k_add_close_tag);
        end_element();  //event
        return m_source.str();
    }

};


//---------------------------------------------------------------------------------------
class ScoreMnxGenerator : public MnxGenerator
{
protected:
    ImoScore* m_pObj;

public:
    ScoreMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
    }

    string generate_source()
    {
        start_element("score", m_pObj);
        close_start_tag();
        add_cwmnx_tag();
        add_styles();
        add_titles();
        add_page_layout();
        add_system_layout();
        add_cursor();
        add_options();
        add_instruments_and_groups();
        end_element();  //cwmnx
        end_element();  //score
        return m_source.str();
    }

protected:

    void add_cwmnx_tag()
    {
        start_element("cwmnx", nullptr);
        add_attribute("profile", "standard");
        close_start_tag();
        add_global();
    }

    void add_global()
    {
        start_element("global", nullptr);
        close_start_tag();
        end_element();
    }

    void add_styles()
    {
//    //styles
//    m_source << m_TextStyles.SourceMNX(1, fUndoData);
    }

    void add_titles()
    {
//    //titles and other attached auxobjs
//    if (m_pAuxObjs)
//    {
//	    for (int i=0; i < (int)m_pAuxObjs->size(); i++)
//	    {
//		    m_source << (*m_pAuxObjs)[i]->SourceMNX(1, fUndoData);
//	    }
//    }
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
//            m_source << (*it)->SourceMNX(1, fUndoData);
//#else
//        lmPageInfo* pPageInfo = m_PagesInfo.front();
//        m_source << pPageInfo->SourceMNX(1, fUndoData);
//#endif
//        //first system and other systems layout info
//        m_source << m_SystemsInfo.front()->SourceMNX(1, true, fUndoData);
//        m_source << m_SystemsInfo.back()->SourceMNX(1, false, fUndoData);
//    }
    }

    void add_system_layout()
    {
    }

    void add_cursor()
    {
//    //score cursor information
//    if (fUndoData)
//    {
//        lmCursorState tState = m_SCursor.GetState();
//        m_source << wxString::Format(_T("   (cursor %d %d %s %d)\n"),
//                        tState.GetNumInstr(),
//                        tState.GetNumStaff(),
//		                DoubleToStr((double)tState.GetTimepos(), 2).c_str(),
//                        tState.GetObjID() );
//    }
    }

    void add_options()
    {
    //    //options with non-default values
    //    bool fBoolValue;
    //    long nLongValue;
    //    double rDoubleValue;
    //
    //    //bool
    //    for (int i=0; i < (int)(sizeof(m_BoolOptions)/sizeof(lmBoolOption)); i++)
    //    {
    //        fBoolValue = GetOptionBool(m_BoolOptions[i].sOptName);
    //        if (fBoolValue != m_BoolOptions[i].fBoolValue)
    //            m_source << wxString::Format(_T("   (opt %s %s)\n"), m_BoolOptions[i].sOptName.c_str(),
    //                                        (fBoolValue ? _T("true") : _T("false")) );
    //    }
    //
    //    //long
    //    for (int i=0; i < (int)(sizeof(m_LongOptions)/sizeof(lmLongOption)); i++)
    //    {
    //        nLongValue = GetOptionLong(m_LongOptions[i].sOptName);
    //        if (nLongValue != m_LongOptions[i].nLongValue)
    //            m_source << wxString::Format(_T("   (opt %s %d)\n"), m_LongOptions[i].sOptName.c_str(),
    //                                        nLongValue );
    //    }
    //
    //    //double
    //    for (int i=0; i < (int)(sizeof(m_DoubleOptions)/sizeof(MNXoubleOption)); i++)
    //    {
    //        rDoubleValue = GetOptionDouble(m_DoubleOptions[i].sOptName);
    //        if (rDoubleValue != m_DoubleOptions[i].rDoubleValue)
    //            m_source << wxString::Format(_T("   (opt %s %s)\n"), m_DoubleOptions[i].sOptName.c_str(),
    //                                        DoubleToStr(rDoubleValue, 4).c_str() );
    //    }
    }

    void add_instruments_and_groups()
    {
        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
            add_source_for( m_pObj->get_instrument(i) );
    }


};


//---------------------------------------------------------------------------------------
class ScoreObjMnxGenerator : public MnxGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjMnxGenerator(ImoObj* pImo, MnxExporter* pExporter)
        : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    string generate_source()
    {
        add_visible();
        add_color();
        source_for_base_contentobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_visible()
    {
        if (!m_pObj->is_visible())
            m_source << " (visible no)";
    }

    void add_color()
    {
        //color (if not black)
        Color color = m_pObj->get_color();
        if (color.r != 0 || color.g != 0  || color.b != 0 || color.a != 255)
            m_source << " (color " << MnxExporter::color_to_mnx(color) << ")";
    }
};


//---------------------------------------------------------------------------------------
class SpacerMnxGenerator : public MnxGenerator
{
protected:
    ImoSpacer* m_pObj;

public:
    SpacerMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoSpacer*>(pImo);
    }

    string generate_source()
    {
        start_element("spacer", m_pObj);
        close_start_tag();
        //TODO: details
        end_element(k_in_same_line);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class StaffObjMnxGenerator : public MnxGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    string generate_source()
    {
        add_staff_num();
        source_for_base_scoreobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_staff_num()
    {
        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
            && !m_pObj->is_time_signature()
            && !m_pObj->is_barline() )
        {
//            m_source << " p" << (m_pObj->get_staff() + 1);
            //m_source << "<staff>" << (m_pObj->get_staff() + 1) << "</staff>";
            start_element("staff", nullptr);
            close_start_tag();
            m_source << (m_pObj->get_staff() + 1);
            end_element(k_in_same_line);
        }
    }

};


//---------------------------------------------------------------------------------------
class StylesMnxGenerator : public MnxGenerator
{
protected:
    ImoStyles* m_pObj;

public:
    StylesMnxGenerator(ImoObj* pImo, MnxExporter* pExporter) : MnxGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStyles*>(pImo);
    }

    string generate_source()
    {
        if (there_is_any_non_default_style())
        {
            start_element("styles", m_pObj);
            close_start_tag();
            add_styles();
            end_element();
            empty_line();
            return m_source.str();
        }
        return "";
    }

protected:

    bool there_is_any_non_default_style()
    {
     	map<std::string, ImoStyle*>& styles = m_pObj->get_styles_collection();

        map<std::string, ImoStyle*>::iterator it;
        for(it = styles.begin(); it != styles.end(); ++it)
        {
            ImoStyle* pStyle = it->second;
            if (!pStyle->is_default_style_with_default_values())
                return true;
        }
        return false;
    }

    void add_styles()
    {
     	map<std::string, ImoStyle*>& styles = m_pObj->get_styles_collection();

        map<std::string, ImoStyle*>::iterator it;
        for(it = styles.begin(); it != styles.end(); ++it)
        {
            ImoStyle* pStyle = it->second;
            if (!pStyle->is_default_style_with_default_values())
                add_source_for(pStyle);
        }
    }

};



//=======================================================================================
// MnxGenerator implementation
//=======================================================================================
MnxGenerator::MnxGenerator(MnxExporter* pExporter)
    : m_pExporter(pExporter)
{
}

//---------------------------------------------------------------------------------------
void MnxGenerator::start_element(const string& name, ImoObj* pImo)
{
    new_line_and_indent_spaces();
    m_source << "<" << name;
    m_pExporter->push_tag(name);
    if (pImo && m_pExporter->get_add_id())
        m_source << " id=\"" << std::dec << pImo->get_id() << "\"";
    increment_indent();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::close_start_tag()
{
    m_source << ">";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::start_attrib(const string& name)
{
    m_source << " " << name << "=\"";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::end_attrib()
{
    m_source << "\"";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::add_attribute(const string& name, const string& value)
{
    start_attrib(name);
    m_source << value;
    end_attrib();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::end_element(bool fInNewLine, bool fNewElement)
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
void MnxGenerator::start_comment()
{
    new_line_and_indent_spaces();
    m_source << "<!-- ";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::end_comment()
{
    m_source << " -->";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::empty_line()
{
    m_source.clear();
    new_line();
}
//---------------------------------------------------------------------------------------
void MnxGenerator::new_line_and_indent_spaces(bool fStartLine)
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
void MnxGenerator::new_line()
{
    if (!m_pExporter->get_remove_newlines())
        m_source << endl;
}

//---------------------------------------------------------------------------------------
void MnxGenerator::add_source_for(ImoObj* pImo)
{
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjMnxGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjMnxGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjMnxGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_base_imobj(ImoObj* pImo)
{
    increment_indent();
    ImObjMnxGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
    decrement_indent();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void MnxGenerator::source_for_inline_level_object(ImoInlineLevelObj* pImo,
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
void MnxGenerator::add_optional_style(ImoContentObj* pObj)
{
    ImoStyle* pStyle = pObj->get_style();
    if (pStyle && !pStyle->is_default_style_with_default_values())
        m_source << " style=\"" << pStyle->get_name() << "\"";
}

//---------------------------------------------------------------------------------------
void MnxGenerator::increment_indent()
{
    m_pExporter->increment_indent();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::decrement_indent()
{
    m_pExporter->decrement_indent();
}

//---------------------------------------------------------------------------------------
void MnxGenerator::add_duration(stringstream& source, int noteType, int dots)
{
    start_attrib("value");
    switch(noteType)
    {
        case k_longa:   source << "*4";  break;
        case k_breve:   source << "*2";  break;
        case k_whole:   source << "/1";  break;
        case k_half:    source << "/2";  break;
        case k_quarter: source << "/4";  break;
        case k_eighth:  source << "/8";  break;
        case k_16th:    source << "/16";  break;
        case k_32nd:    source << "/32";  break;
        case k_64th:    source << "/64";  break;
        case k_128th:   source << "/128";  break;
        case k_256th:   source << "/256";  break;
        default:                        break;
    }

    while (dots > 0)
    {
        source << "d";
        --dots;
    }
    end_attrib();
}


//=======================================================================================
// MnxExporter implementation
//=======================================================================================
MnxExporter::MnxExporter(LibraryScope& libScope)
    : m_libraryScope(libScope)
    , m_nIndent(0)
    , m_fAddId(false)
    , m_fRemoveNewlines(false)
    , m_fProcessingChord(false)
{
    m_lomseVersion = libScope.get_version_string();
    m_exportTime = to_simple_string(chrono::system_clock::now());
}

//---------------------------------------------------------------------------------------
MnxExporter::~MnxExporter()
{
}

//---------------------------------------------------------------------------------------
string MnxExporter::get_version_and_time_string()
{
    string value = m_lomseVersion;
    value.append(", date ");
    value.append(m_exportTime);
    return value;
}

//---------------------------------------------------------------------------------------
string MnxExporter::get_source(ImoObj* pImo)
{
    MnxGenerator* pGen = new_generator(pImo);
    string source = pGen->generate_source();
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
MnxGenerator* MnxExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_barline:         return LOMSE_NEW BarlineMnxGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefMnxGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocMnxGenerator(pImo, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentMnxGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureMnxGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataMnxGenerator(pImo, this);
        case k_imo_note:            return LOMSE_NEW NoteMnxGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW RestMnxGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreMnxGenerator(pImo, this);
        case k_imo_spacer:          return LOMSE_NEW SpacerMnxGenerator(pImo, this);
        case k_imo_style:           return LOMSE_NEW DefineStyleMnxGenerator(pImo, this);
        case k_imo_styles:          return LOMSE_NEW StylesMnxGenerator(pImo, this);
        default:
            return new ErrorMnxGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
string MnxExporter::clef_type_to_mnx(int clefType)
{
    //AWARE: indexes in correspondence with enum k_clef__type
    static const string name[] = {
        "G2",
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
string MnxExporter::barline_type_to_mnx(int barType)
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
string MnxExporter::color_to_mnx(Color color)
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
string MnxExporter::float_to_string(float UNUSED(num))
{
    return "(TODO: float_to_string)";
}



}  //namespace lomse
