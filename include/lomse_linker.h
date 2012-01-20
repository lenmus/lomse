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

#ifndef __LOMSE_LINKER_H__
#define __LOMSE_LINKER_H__

#include <iostream>

using namespace std;

namespace lomse
{

//forward declarations
class Document;
class ImoAuxObj;
class ImoBezierInfo;
class ImoContent;
class ImoCursorInfo;
class ImoFontStyleDto;
class ImoInstrGroup;
class ImoInstrument;
class ImoListItem;
class ImoMidiInfo;
class ImoObj;
class ImoOptionInfo;
class ImoPageInfo;
class ImoParamInfo;
class ImoStaffObj;
class ImoSystemInfo;
class ImoScoreText;
class ImoScoreTitle;
class ImoStaffInfo;
class ImoTextItem;
class ImoStyle;


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
    Linker(Document* pDoc) : m_pDoc(pDoc) {}
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
    ImoObj* add_midi_info(ImoMidiInfo* pInfo);
    ImoObj* add_param_info(ImoParamInfo* pParam);
    ImoObj* add_staff_info(ImoStaffInfo* pInfo);
    ImoObj* add_instrument(ImoInstrument* pInstrument);
    ImoObj* add_text(ImoScoreText* pText);
    ImoObj* add_text_item(ImoTextItem* pText);
    ImoObj* add_title(ImoScoreTitle* pTitle);
    ImoObj* add_style(ImoStyle* pStyle);
    ImoObj* add_staffobj(ImoStaffObj* pSO);
    ImoObj* add_attachment(ImoAuxObj* pAuxObj);
    ImoObj* add_child(int parentType, ImoObj* pImo);
    ImoObj* add_font_style(ImoFontStyleDto* pDto);

};


}   //namespace lomse

#endif      //__LOMSE_LINKER_H__
