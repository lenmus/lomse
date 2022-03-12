//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LINKER_H__
#define __LOMSE_LINKER_H__

#include <iostream>

using namespace std;

namespace lomse
{

//forward declarations
class Document;
class ImoAnonymousBlock;
class ImoAuxObj;
class ImoBezierInfo;
class ImoBlockLevelObj;
class ImoContent;
class ImoCursorInfo;
class ImoFontStyleDto;
class ImoInlineLevelObj;
class ImoInstrGroup;
class ImoInstrument;
class ImoSoundInfo;
class ImoListItem;
class ImoSoundInfo;
class ImoObj;
class ImoOptionInfo;
class ImoPageInfo;
class ImoParamInfo;
class ImoRelObj;
class ImoScore;
class ImoScoreText;
class ImoScoreTitle;
class ImoSoundChange;
class ImoStaffInfo;
class ImoStaffObj;
class ImoStyle;
class ImoSystemInfo;
class ImoTextItem;


//---------------------------------------------------------------------------------------
//Linker: responsible for code generation phase, step 1: links ImObjs in the
//parse tree to create the tree of ImObjs. After this step the parse tree could
//be discarded, as all structure and info is captured by the ImObj tree.
class Linker
{
protected:
    Document* m_pDoc;
    ImoObj* m_pParent;
    int m_ldpChildType;

public:
    Linker(Document* pDoc)
        : m_pDoc(pDoc)
        , m_pParent(nullptr)
        , m_ldpChildType(0)
    {
    }
    ~Linker() {}

    ImoObj* add_child_to_model(ImoObj* pParent, ImoObj* pChild, int ldpChildType);

protected:
    ImoObj* add_content(ImoContent* pContent);
    ImoObj* add_instruments_group(ImoInstrGroup* pGrp);
    ImoObj* add_option(ImoOptionInfo* pOpt);
    ImoObj* add_page_info(ImoPageInfo* pPI);
    ImoObj* add_system_info(ImoSystemInfo* pSI);
    ImoObj* add_bezier(ImoBezierInfo* pBezier);
    ImoObj* add_cursor(ImoCursorInfo* pCursor);
    ImoObj* add_listitem(ImoListItem* pItem);
    ImoObj* add_sound_change(ImoSoundChange* pInfo);
    ImoObj* add_sound_info(ImoSoundInfo* pInfo);
    ImoObj* add_param_info(ImoParamInfo* pParam);
    ImoObj* add_staff_info(ImoStaffInfo* pInfo);
    ImoObj* add_instrument(ImoInstrument* pInstrument);
    ImoObj* add_text(ImoScoreText* pText);
    //ImoObj* add_text_item(ImoTextItem* pText);
    ImoObj* add_title(ImoScoreTitle* pTitle);
    ImoObj* add_style(ImoStyle* pStyle);
    ImoObj* add_staffobj(ImoStaffObj* pSO);
    ImoObj* add_attachment(ImoAuxObj* pAuxObj);
    ImoObj* add_relation(ImoRelObj* pRelObj);
    ImoObj* add_child(int parentType, ImoObj* pImo);
    ImoObj* add_font_style(ImoFontStyleDto* pDto);
    //ImoObj* add_inline_or_block_item(ImoInlineLevelObj* pImo);
    ImoObj* add_inline_level_item(ImoInlineLevelObj* pImo);
    ImoObj* add_block_level_item(ImoBlockLevelObj* pImo);

};


}   //namespace lomse

#endif      //__LOMSE_LINKER_H__
