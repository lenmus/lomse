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

#ifndef __LOMSE_LINKER_H__ 
#define __LOMSE_LINKER_H__

#include <iostream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoAuxObj;
class ImoBezierInfo;
class ImoInstrGroup;
class ImoInstrument;
class ImoMidiInfo;
class ImoObj;
class ImoOptionInfo;
class ImoStaffObj;
class ImoSystemInfo;
class ImoTextString;


//Linker: responsible for code generation phase, step 1: links ImObjs in the
//parse tree to create the tree of ImObjs. After this step the parse tree could 
//be discarded, as all structure and info is captured by the ImObj tree.
//--------------------------------------------------------------------------------
class Linker
{
public:
    Linker() {}
    ~Linker() {}

    ImoObj* add_child_to_model(ImoObj* pParent, ImoObj* pChild, int ldpChildType);

protected:
    ImoObj* m_pParent;
    int m_ldpChildType;

    ImoObj* add_instruments_group(ImoInstrGroup* pGrp);
    ImoObj* add_option(ImoOptionInfo* pOpt);
    ImoObj* add_system_info(ImoSystemInfo* pSI);
    ImoObj* add_bezier(ImoBezierInfo* pBezier);
    ImoObj* add_midi_info(ImoMidiInfo* pInfo);
    ImoObj* add_instrument(ImoInstrument* pInstrument);
    ImoObj* add_text(ImoTextString* pText);
    ImoObj* add_staffobj(ImoStaffObj* pSO);
    ImoObj* add_auxobj(ImoAuxObj* pAuxObj);
    ImoObj* add_child(int parentType, ImoObj* pImo);

};


}   //namespace lomse

#endif      //__LOMSE_LINKER_H__
