//---------------------------------------------------------------------------------------
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
//---------------------------------------------------------------------------------------

#include "lomse_ldp_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"


namespace lomse
{

#define lml_LDP_INDENT_STEP  3

//=======================================================================================
// LdpGenerator
//=======================================================================================
class LdpGenerator
{
protected:
    LdpExporter* m_pExporter;
    stringstream m_source;

public:
    LdpGenerator(LdpExporter* pExporter) : m_pExporter(pExporter) {}

    virtual std::string generate_source() = 0;

protected:
    void start_element();
    void end_element();
    void add_indent_spaces();
    void add_element_name(const std::string& name, ImoObj* pImo);
    void add_source_for(ImoObj* pImo);
    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void increment_indent();
    void decrement_indent();

};



//=======================================================================================
// generators for specific elements
//=======================================================================================


//---------------------------------------------------------------------------------------
class BarlineLdpGenerator : public LdpGenerator
{
protected:
    ImoBarline* m_pObj;

public:
    BarlineLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = dynamic_cast<ImoBarline*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("barline", m_pObj);
//        add_barline_type();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ClefLdpGenerator : public LdpGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = dynamic_cast<ImoClef*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("clef", m_pObj);
        add_type();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_type()
    {
        m_source << LdpExporter::clef_type_to_ldp( m_pObj->get_clef_type() );
    }

};


//---------------------------------------------------------------------------------------
class ScoreObjLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = dynamic_cast<ImoScoreObj*>(pImo);
    }

    std::string generate_source()
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
        if (color != Color(0,0,0))
            m_source << " (color " << LdpExporter::color_to_ldp(color) << ")";
    }
};


//---------------------------------------------------------------------------------------
class ContentObjLdpGenerator : public LdpGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = dynamic_cast<ImoContentObj*>(pImo);
    }

    std::string generate_source()
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
            m_source << " (dx " << LdpExporter::float_to_string(ux) << ")";

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
            m_source << " (dy " << LdpExporter::float_to_string(uy) << ")";
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
//                    //exclude beams, as source code for them is generted in lmNote.
//                    //AWARE. This is necessary because LDP parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if ( pRO->GetStartNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLDP_First(nIndent, fUndoData, (lmNoteRest*)this);
//                        else if ( pRO->GetEndNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLDP_Last(nIndent, fUndoData, (lmNoteRest*)this);
//                        else
//                            m_source += pRO->SourceLDP_Middle(nIndent, fUndoData, (lmNoteRest*)this);
//                    }
//                }
//                else if ( pAuxObj->IsRelObX() )
//                {
//                    lmRelObX* pRO = (lmRelObX*)pAuxObj;
//
//                    //exclude beams, as source code for them is generted in lmNote.
//                    //AWARE. This is necessary because LDP parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if (pRO->GetStartSO() == this)
//                            m_source += pRO->SourceLDP_First(nIndent, fUndoData, this);
//                        else if (pRO->GetEndSO() == this)
//                            m_source += pRO->SourceLDP_Last(nIndent, fUndoData, this);
//                        else
//                            m_source += pRO->SourceLDP_Middle(nIndent, fUndoData, this);
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
class ErrorLdpGenerator : public LdpGenerator
{
public:
    ErrorLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter) {}

    std::string generate_source()
    {
        m_source.clear();
        m_source << "(TODO: Add this element to LdpExporter::new_generator)";
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ImObjLdpGenerator : public LdpGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = pImo;
    }

    std::string generate_source()
    {
        return m_source.str();
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
        m_pObj = dynamic_cast<ImoDocument*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("lenmusdoc", m_pObj);
        add_version();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    void add_version()
    {
        m_source << "(vers ";
        m_source << m_pObj->get_version();
        m_source << ") ";
    }

    void add_content()
    {
        m_source << "(content";
        int numItems = m_pObj->get_num_content_items();
        for (int i=0; i < numItems; i++)
        {
            m_source << " ";
            add_source_for( m_pObj->get_content_item(i) );
        }
        m_source << ")";
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
        m_pObj = dynamic_cast<ImoNote*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("n", m_pObj);
        add_pitch();
        add_duration();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_pitch()
    {
        m_source << " (TODO: pitch)";
    }

    void add_duration()
    {
        m_source << " (TODO: duration)";
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
        m_pObj = dynamic_cast<ImoRest*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("r", m_pObj);
        add_duration();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_duration()
    {
        m_source << " (TODO: duration)";
    }

};


//---------------------------------------------------------------------------------------
class StaffObjLdpGenerator : public LdpGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = dynamic_cast<ImoStaffObj*>(pImo);
    }

    std::string generate_source()
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
            m_source << " p" << (m_pObj->get_staff() + 1);
        }
    }

};



//=======================================================================================
// LdpGenerator implementation
//=======================================================================================
void LdpGenerator::start_element()
{
    add_indent_spaces();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_element()
{
    m_source << ")";
    if (m_pExporter->get_indent() > 0)
        m_source << endl;
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_indent_spaces()
{
    m_source.clear();
    int indent = m_pExporter->get_indent() * lml_LDP_INDENT_STEP;
    while (indent > 0)
    {
        m_source << " ";
        indent--;
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_element_name(const std::string& name, ImoObj* pImo)
{
    m_source << "(" << name;
    if (m_pExporter->get_add_id())
        m_source << "#" << std::dec << pImo->get_id();
    m_source << " ";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_source_for(ImoObj* pImo)
{
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_imobj(ImoObj* pImo)
{
    ImObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::increment_indent()
{
    //TODO
}

//---------------------------------------------------------------------------------------
void LdpGenerator::decrement_indent()
{
    //TODO
}



//=======================================================================================
// LdpExporter implementation
//=======================================================================================
LdpExporter::LdpExporter()
    : m_nIndent(0)
    , m_fAddId(false)
{
}

//---------------------------------------------------------------------------------------
LdpExporter::~LdpExporter()
{
}

//---------------------------------------------------------------------------------------
std::string LdpExporter::get_source(ImoObj* pImo)
{
    LdpGenerator* pGen = new_generator(pImo);
    std::string source = pGen->generate_source();
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
LdpGenerator* LdpExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case ImoObj::k_barline:         return new BarlineLdpGenerator(pImo, this);
//        case ImoObj::k_beam_dto:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_bezier_info:         return new XxxxxxxLdpGenerator(pImo, this);
        case ImoObj::k_clef:            return new ClefLdpGenerator(pImo, this);
//        case ImoObj::k_color_dto:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_instr_group:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_midi_info:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_option:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_system_info:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_tie_dto:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_tuplet_dto:         return new XxxxxxxLdpGenerator(pImo, this);
        case ImoObj::k_document:        return new LenmusdocLdpGenerator(pImo, this);
//        case ImoObj::k_content:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_music_data:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_instrument:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_score:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_key_signature:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_time_signature:         return new XxxxxxxLdpGenerator(pImo, this);
        case ImoObj::k_note:            return new NoteLdpGenerator(pImo, this);
        case ImoObj::k_rest:            return new RestLdpGenerator(pImo, this);
//        case ImoObj::k_go_back_fwd:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_metronome_mark:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_control:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_spacer:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_figured_bass:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_score_text:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_fermata:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_tie:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_beam:         return new XxxxxxxLdpGenerator(pImo, this);
//        case ImoObj::k_tuplet:         return new XxxxxxxLdpGenerator(pImo, this);
        default:
            return new ErrorLdpGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
std::string LdpExporter::clef_type_to_ldp(int clefType)
{
    //AWARE: indexes in correspondence with enum ImoClef::k__type
    static const std::string name[] = {
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
    static const std::string undefined = "undefined";


    if (clefType == ImoClef::k_undefined)
        return undefined;
    else
        return name[clefType];
}

//---------------------------------------------------------------------------------------
std::string LdpExporter::color_to_ldp(Color& color)
{
    stringstream source;
    source << "#";
    source << std::hex << setfill('0') << setw(2) << color.r;
    source << std::hex << setfill('0') << setw(2) << color.g;
    source << std::hex << setfill('0') << setw(2) << color.b;
    source << std::hex << setfill('0') << setw(2) << color.a;
    return source.str();
}

//---------------------------------------------------------------------------------------
std::string LdpExporter::float_to_string(float num)
{
    return "(TODO: float_to_string)";
}



}  //namespace lomse
