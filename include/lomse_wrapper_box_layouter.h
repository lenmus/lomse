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

#ifndef __LOMSE_WRAPPER_BOX_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_WRAPPER_BOX_LAYOUTER_H__

#include "lomse_injectors.h"
#include "lomse_content_layouter.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoStyles;
class ImoStyle;
class GraphicModel;
class GmoBox;
class ContentLayouter;

//---------------------------------------------------------------------------------------
// WrapperBoxLayouter: layouts a paragraph
class WrapperBoxLayouter : public ContentLayouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoStyles* m_pStyles;
    GmoBox* m_pMainBox;

public:
    WrapperBoxLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                       LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~WrapperBoxLayouter();

};


}   //namespace lomse

#endif    // __LOMSE_WRAPPER_BOX_LAYOUTER_H__

