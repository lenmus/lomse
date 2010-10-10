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
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_DOCUMENT_LAYOUTER_H__

#include "lomse_basic.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class GraphicModel;
class ImoDocObj;
class ContentLayouter;


// DocLayouter: Generates LDP source code for a basic model object
//----------------------------------------------------------------------------------
class DocLayouter
{
protected:
    InternalModel* m_pIModel;
    GraphicModel* m_pGModel;

public:
    DocLayouter(InternalModel* pIModel);
    virtual ~DocLayouter();

    void layout_document(USize pagesize);
    inline GraphicModel* get_gm_model() { return m_pGModel; }


protected:
    void initializations(USize pagesize);
    void assign_space();
    void layout_content();
    ContentLayouter* new_item_layouter(ImoDocObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE_DOCUMENT_LAYOUTER_H__

