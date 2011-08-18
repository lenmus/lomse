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

#include "lomse_linker.h"

#include "lomse_internal_model.h"
#include "lomse_ldp_elements.h"        //for node type
#include "lomse_im_note.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"


using namespace std;

namespace lomse
{


//---------------------------------------------------------------------------------------
ImoObj* Linker::add_child_to_model(ImoObj* pParent, ImoObj* pChild, int ldpChildType)
{
    //If the object (or its content, for DTOs) is added to the model it must return NULL.
    //Othewise, it must return the received object. This behaviour is necessary to
    //simplify unit tests of Analyser

    m_ldpChildType = ldpChildType;
    m_pParent = pParent;

    switch(pChild->get_obj_type())
    {
        case k_imo_bezier_info:
            return add_bezier(static_cast<ImoBezierInfo*>(pChild));

        case k_imo_content:
            return add_content(static_cast<ImoContent*>(pChild));

        case k_imo_cursor_info:
            return add_cursor(static_cast<ImoCursorInfo*>(pChild));

        case k_imo_font_style_dto:
            return add_font_style(static_cast<ImoFontStyleDto*>(pChild));

        case k_imo_instrument:
            return add_instrument(static_cast<ImoInstrument*>(pChild));

        case k_imo_instr_group:
            return add_instruments_group(static_cast<ImoInstrGroup*>(pChild));

        case k_imo_midi_info:
            return add_midi_info(static_cast<ImoMidiInfo*>(pChild));

        case k_imo_music_data:
            return add_child(k_imo_instrument, pChild);

        case k_imo_option:
            return add_option(static_cast<ImoOptionInfo*>(pChild));

        case k_imo_page_info:
            return add_page_info(static_cast<ImoPageInfo*>(pChild));

        case k_imo_param_info:
            return add_param_info(static_cast<ImoParamInfo*>(pChild));

        case k_imo_score_text:
            return add_text(static_cast<ImoScoreText*>(pChild));

        case k_imo_score_title:
            return add_title(static_cast<ImoScoreTitle*>(pChild));

        case k_imo_staff_info:
            return add_staff_info(static_cast<ImoStaffInfo*>(pChild));

        case k_imo_styles:
            return add_child(k_imo_document, pChild);

        case k_imo_system_info:
            return add_system_info(static_cast<ImoSystemInfo*>(pChild));

        case k_imo_text_item:
            return add_text_item(static_cast<ImoTextItem*>(pChild));

        case k_imo_style:
            return add_style(static_cast<ImoStyle*>(pChild));

        default:
            if (pChild->is_box_container() || pChild->is_box_content())
                return add_child(k_imo_content, pChild);
            else if (pChild->is_staffobj())
                return add_staffobj(static_cast<ImoStaffObj*>(pChild));
            else if (pChild->is_auxobj())
                return add_attachment(static_cast<ImoAuxObj*>(pChild));
            else
                return pChild;
    }
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_content(ImoContent* pContent)
{
    if (m_pParent && (m_pParent->is_document() || m_pParent->is_box_container()))
    {
        m_pParent->append_child(pContent);
        return NULL;
    }
    return pContent;
}


//---------------------------------------------------------------------------------------
ImoObj* Linker::add_instruments_group(ImoInstrGroup* pGrp)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(m_pParent);
        pScore->add_instruments_group(pGrp);
    }
    return pGrp;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_option(ImoOptionInfo* pOpt)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>( m_pParent );
        pScore->add_option(pOpt);
        return NULL;
    }
    return pOpt;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_page_info(ImoPageInfo* pPI)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(m_pParent);
        pScore->add_page_info(pPI);
        delete pPI;
        return NULL;
    }
    else if (m_pParent && m_pParent->is_document())
    {
        ImoDocument* pDoc = static_cast<ImoDocument*>(m_pParent);
        pDoc->add_page_info(pPI);
        delete pPI;
        return NULL;
    }
    return pPI;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_cursor(ImoCursorInfo* pCursor)
{
    if (m_pParent && m_pParent->is_document())
    {
        ImoDocument* pDoc = static_cast<ImoDocument*>(m_pParent);
        pDoc->add_cursor_info(pCursor);
        delete pCursor;
        return NULL;
    }
    return pCursor;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_system_info(ImoSystemInfo* pSI)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(m_pParent);
        pScore->add_sytem_info(pSI);
        delete pSI;
        return NULL;
    }
    return pSI;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_style(ImoStyle* pStyle)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(m_pParent);
        pScore->add_style(pStyle);
        return NULL;
    }
    else if (m_pParent && m_pParent->is_styles())
    {
        ImoStyles* pStyles = static_cast<ImoStyles*>(m_pParent);
        pStyles->add_style(pStyle);
        return NULL;
    }
    return pStyle;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_bezier(ImoBezierInfo* pBezier)
{
    if (m_pParent && m_pParent->is_tie_dto())
    {
        ImoTieDto* pInfo = static_cast<ImoTieDto*>(m_pParent);
        pInfo->set_bezier(pBezier);
        return NULL;
    }
    else if (m_pParent && m_pParent->is_slur_dto())
    {
        ImoSlurDto* pInfo = static_cast<ImoSlurDto*>(m_pParent);
        pInfo->set_bezier(pBezier);
        return NULL;
    }
    return pBezier;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_midi_info(ImoMidiInfo* pInfo)
{
    if (m_pParent && m_pParent->is_instrument())
    {
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pParent);
        pInstr->set_midi_info(pInfo);
        return NULL;
    }
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_param_info(ImoParamInfo* pParam)
{
    if (m_pParent && m_pParent->is_dynamic())
    {
        ImoDynamic* pDyn = static_cast<ImoDynamic*>(m_pParent);
        pDyn->add_param(pParam);
        return NULL;
    }
    return pParam;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_staff_info(ImoStaffInfo* pInfo)
{
    if (m_pParent && m_pParent->is_instrument())
    {
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pParent);
        pInstr->replace_staff_info(pInfo);
        return NULL;
    }
    return pInfo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_instrument(ImoInstrument* pInstrument)
{
    if (m_pParent)
    {
        if (m_pParent->is_instr_group())
        {
            ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>( m_pParent );
            pGrp->add_instrument(pInstrument);
        }
        else if (m_pParent->is_score())
        {
            ImoScore* pScore = static_cast<ImoScore*>( m_pParent );
            pScore->add_instrument(pInstrument);
        }
    }
    return pInstrument;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_text(ImoScoreText* pText)
{
    if (m_pParent)
    {
        //compatibility with 1.5. Since 1.6 auxObjs can not be included
        //in musicData; they must go attached to an spacer.
        if (m_pParent->is_music_data())
        {
            //musicData: create anchor (ImoSpacer) and attach to it
            ImoSpacer* pSpacer = static_cast<ImoSpacer*>(
                                        ImFactory::inject(k_imo_spacer, m_pDoc) );
            pSpacer->add_attachment(m_pDoc, pText);
            add_staffobj(pSpacer);
            return pText;
        }

        if (m_pParent->is_instrument())
        {
            ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pParent);
            //could be 'name' or 'abbrev'
            if (m_ldpChildType == k_name)
                pInstr->set_name(pText);
            else
                pInstr->set_abbrev(pText);
            return NULL;
        }

        if (m_pParent->is_instr_group())
        {
            ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(m_pParent);
            //could be 'name' or 'abbrev'
            if (m_ldpChildType == k_name)
                pGrp->set_name(pText);
            else
                pGrp->set_abbrev(pText);
            return NULL;
        }

        if (m_pParent->is_content())
        {
            add_child(k_imo_content, pText);
            return pText;
        }

        return add_attachment(pText);
    }
    return pText;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_text_item(ImoTextItem* pText)
{
    if (m_pParent)
    {
        if (m_pParent->is_textblock())
        {
            ImoTextBlock* pBox = static_cast<ImoTextBlock*>(m_pParent);
            pBox->add_item(pText);
            return NULL;
        }

        if (m_pParent->is_content())
        {
            add_child(k_imo_content, pText);
            return NULL;
        }
    }
    return pText;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_title(ImoScoreTitle* pTitle)
{
    if (m_pParent && m_pParent->is_score())
    {
        ImoScore* pScore = static_cast<ImoScore*>(m_pParent);
        pScore->add_title(pTitle);
    }
    return pTitle;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_child(int parentType, ImoObj* pImo)
{
    if (m_pParent && m_pParent->get_obj_type() == parentType)
        m_pParent->append_child(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_staffobj(ImoStaffObj* pSO)
{
    if (m_pParent)
    {
        if (m_pParent->is_music_data())
            m_pParent->append_child(pSO);
        else if (m_pParent->is_chord() && pSO->is_note())
        {
            ImoChord* pChord = static_cast<ImoChord*>(m_pParent);
            ImoNote* pNote = static_cast<ImoNote*>(pSO);
            pNote->include_in_relation(m_pDoc, pChord);
            return NULL;
        }
    }
    return pSO;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_attachment(ImoAuxObj* pAuxObj)
{
    if (m_pParent && m_pParent->is_contentobj())
    {
        ImoContentObj* pContentObj = static_cast<ImoContentObj*>(m_pParent);
        pContentObj->add_attachment(m_pDoc, pAuxObj);
    }
    return pAuxObj;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_font_style(ImoFontStyleDto* pDto)
{
    if (m_pParent && m_pParent->is_style())
    {
        ImoStyle* pStyle = static_cast<ImoStyle*>(m_pParent);
        pStyle->set_float_property(ImoStyle::k_font_size, pDto->size);
        pStyle->set_int_property(ImoStyle::k_font_weight, pDto->weight);
        pStyle->set_int_property(ImoStyle::k_font_style, pDto->style);
        pStyle->set_string_property(ImoStyle::k_font_name, pDto->name);
        delete pDto;
        return NULL;
    }
    return pDto;
}


}   //namespace lomse
