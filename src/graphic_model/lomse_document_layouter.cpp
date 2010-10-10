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

#include "lomse_document_layouter.h"

#include "lomse_basic_model.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_content_layouter.h"
#include "lomse_score_layouter.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_im_note.h"


namespace lomse
{

//-------------------------------------------------------------------------------------
// DocLayouter implementation
//-------------------------------------------------------------------------------------

DocLayouter::DocLayouter(InternalModel* pIModel)
    : m_pIModel(pIModel)
    , m_pGModel(NULL)
{
}

DocLayouter::~DocLayouter()
{
}

void DocLayouter::layout_document(USize pagesize)
{
    // USize pagesize - logical page size 
    // float scale - conversion factor: pixels = LUnits * scale

    initializations(pagesize);
    assign_space();
    layout_content();
}

void DocLayouter::initializations(USize pagesize)
{
    m_pGModel = new GraphicModel();
}

void DocLayouter::assign_space()
{
}

void DocLayouter::layout_content()
{
    ImoDocument* pDoc = dynamic_cast<ImoDocument*>( m_pIModel->get_root() );
    ImoContent* pContent = pDoc->get_content();
    int numItems = pContent->get_num_items();
    for (int i=0; i < numItems; i++)
    {
        ContentLayouter* pCL = new_item_layouter( pContent->get_item(i) );
        GmoBox* pContainerBox = NULL;   //m_pGModel->add_box();
        pCL->do_layout(pContainerBox);
        delete pCL;
    }
}

ContentLayouter* DocLayouter::new_item_layouter(ImoDocObj* pImo)
{
    //factory method

    switch (pImo->get_obj_type())
    {
        case ImoObj::k_score:   return new ScoreLayouter(pImo);
        default:
            return new NullLayouter(pImo);
    }
}


}  //namespace lomse
