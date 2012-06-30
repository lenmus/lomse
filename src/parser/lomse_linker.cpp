//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
    //simplify unit tests of LdpAnalyser

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

        case k_imo_listitem:
            return add_listitem(static_cast<ImoListItem*>(pChild));

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

        case k_imo_style:
            return add_style(static_cast<ImoStyle*>(pChild));

        case k_imo_table_row:
        {
            if (m_pParent)
            {
                if (m_pParent->is_table_head())
                    return add_child(k_imo_table_head, pChild);
                else if (m_pParent->is_table_body())
                    return add_child(k_imo_table_body, pChild);
                else
                    return pChild;
            }
            else
                return pChild;
        }

        case k_imo_table_head:
        case k_imo_table_body:
        {
            if (m_pParent && m_pParent->is_table())
                return add_child(k_imo_table, pChild);
            else
                return pChild;
        }

        default:
            if (pChild->is_block_level_obj())
                return add_block_level_item(static_cast<ImoBlockLevelObj*>(pChild));
            else if (pChild->is_inline_level_obj())
                return add_inline_level_item(static_cast<ImoInlineLevelObj*>(pChild));
            else if (pChild->is_staffobj())
                return add_staffobj(static_cast<ImoStaffObj*>(pChild));
            else if (pChild->is_relobj())
                return add_relation(static_cast<ImoRelObj*>(pChild));
            else if (pChild->is_auxobj())
                return add_attachment(static_cast<ImoAuxObj*>(pChild));
            else
                return pChild;
    }
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_content(ImoContent* pContent)
{
    if (m_pParent && (m_pParent->is_document() || m_pParent->is_blocks_container()))
    {
        m_pParent->append_child_imo(pContent);
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
        pScore->add_or_replace_option(pOpt);
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
ImoObj* Linker::add_listitem(ImoListItem* pItem)
{
    if (m_pParent && m_pParent->is_list())
        return add_block_level_item(pItem);
    else
        return pItem;
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
ImoObj* Linker::add_inline_level_item(ImoInlineLevelObj* pImo)
{
    if (m_pParent)
    {
        if (m_pParent->is_inlines_container())
        {
            ImoInlinesContainer* pBox = static_cast<ImoInlinesContainer*>(m_pParent);
            pBox->add_item(pImo);
            return NULL;
        }

        if (m_pParent->is_blocks_container())
        {
            ImoBlocksContainer* pParent = static_cast<ImoBlocksContainer*>(m_pParent);
            ImoAnonymousBlock* pCurBlock =
                dynamic_cast<ImoAnonymousBlock*>( pParent->get_last_content_item() );
            if (!pCurBlock)
            {
                pCurBlock = static_cast<ImoAnonymousBlock*>(
                                ImFactory::inject(k_imo_anonymous_block, m_pDoc) );
                pParent->append_content_item(pCurBlock);
            }
            pCurBlock->add_item(pImo);
            return NULL;
        }
    }
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_block_level_item(ImoBlockLevelObj* pImo)
{
    if (m_pParent)
    {
        if (m_pParent->is_content())
        {
            m_pParent->append_child_imo(pImo);
            return NULL;
        }
        else if (m_pParent->is_blocks_container())
        {
            ImoBlocksContainer* pParent = static_cast<ImoBlocksContainer*>(m_pParent);
            pParent->append_content_item(pImo);
            return NULL;
        }
    }
    return pImo;
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
        m_pParent->append_child_imo(pImo);
    return pImo;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_staffobj(ImoStaffObj* pSO)
{
    if (m_pParent)
    {
        if (m_pParent->is_music_data())
            m_pParent->append_child_imo(pSO);
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
#if (LOMSE_COMPATIBILITY_1_5 == 1)
    //backwards compatibility with 1.5
    //Until v.1.5 included, it was ok to include AuxObj in a musicData element, and
    //the parser will create an anchor object for it. Since v1.6 the anchor object *must*
    //be explicitly defined in LDP source.
    else if (m_pParent && m_pParent->is_music_data())
    {
        if (pAuxObj->is_score_line())
        {
            //auxObj in musicData: create anchor (ImoSpacer) and attach to it
            ImoSpacer* pSpacer = static_cast<ImoSpacer*>(
                                        ImFactory::inject(k_imo_spacer, m_pDoc) );
            pSpacer->add_attachment(m_pDoc, pAuxObj);
            add_staffobj(pSpacer);
            return NULL;
        }
        else
            return pAuxObj;
    }
#endif  //(LOMSE_COMPATIBILITY_1_5 == 1)

    return pAuxObj;
}

//---------------------------------------------------------------------------------------
ImoObj* Linker::add_relation(ImoRelObj* pRelObj)
{
    if (m_pParent && m_pParent->is_staffobj())
    {
        ImoStaffObj* pSO = static_cast<ImoStaffObj*>(m_pParent);
        pSO->add_relation(m_pDoc, pRelObj);
        return NULL;
    }
    return pRelObj;
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
