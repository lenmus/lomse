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

#include "lomse_document_layouter.h"

#include "lomse_basic_model.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_content_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_sizers.h"
#include "lomse_calligrapher.h"
//#include <iostream>
//#include <iomanip>
//#include "lomse_im_note.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// DocLayouter implementation
//---------------------------------------------------------------------------------------

DocLayouter::DocLayouter(InternalModel* pIModel, LibraryScope& libraryScope)
    : m_pIModel(pIModel)
    , m_pGModel(NULL)
    , m_libraryScope(libraryScope)
    //, m_pMainSizer( new FlowSizer(FlowSizer::k_vertical) )
    , m_pCurrentBox(NULL)
    , m_pCurLayouter(NULL)
{
}

//---------------------------------------------------------------------------------------
DocLayouter::~DocLayouter()
{
    //for unit tests last layouter has been saved. Delete it when the document is deleted
    delete m_pCurLayouter;
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_document()
{
    initializations();
    //////add_content_to_main_sizer();
    //////assign_space_to_content_items();
    layout_content();
    m_pGModel->set_ready(true);
}

//---------------------------------------------------------------------------------------
void DocLayouter::initializations()
{
    m_pGModel = new GraphicModel();
    start_new_document_page();
}

//---------------------------------------------------------------------------------------
void DocLayouter::start_new_document_page()
{
    GmoBoxDocPage* pPage = create_document_page();
    add_margins_to_page(pPage);
    add_headers_to_page(pPage);
    add_footers_to_page(pPage);
    add_contents_wrapper_box_to_page(pPage);
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* DocLayouter::create_document_page()
{
    GmoBoxDocument* pBoxDoc = m_pGModel->get_root();
    GmoBoxDocPage* pPage = pBoxDoc->add_new_page();
    assign_paper_size_to(pPage);
    return pPage;
}

//---------------------------------------------------------------------------------------
void DocLayouter::assign_paper_size_to(GmoBox* pBox)
{
    ImoDocument* pDoc = get_document();
    m_availableWidth = pDoc->get_paper_width();
    m_availableHeight = pDoc->get_paper_height();
    pBox->set_width(m_availableWidth);
    pBox->set_height(m_availableHeight);
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_margins_to_page(GmoBoxDocPage* pPage)
{
    ImoDocument* pDoc = get_document();
    ImoPageInfo* pInfo = pDoc->get_page_info();
    LUnits top = pInfo->get_top_margin();
    LUnits bottom = pInfo->get_bottom_margin();
    LUnits left = pInfo->get_left_margin();
    LUnits right = pInfo->get_right_margin();
    if (pPage->get_number() % 2 == 0)
        left += pInfo->get_binding_margin();
    else
        right += pInfo->get_binding_margin();

    m_pageCursor.x = left;
    m_pageCursor.y = top;
    m_availableWidth -= left + right;
    m_availableHeight -= top + bottom;
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_headers_to_page(GmoBoxDocPage* pPage)
{
    //TODO
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_footers_to_page(GmoBoxDocPage* pPage)
{
    //TODO
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_contents_wrapper_box_to_page(GmoBoxDocPage* pPage)
{
    m_pCurrentBox = new GmoBoxDocPageContent();
    pPage->add_child_box(m_pCurrentBox);
    m_pCurrentBox->set_origin(m_pageCursor.x, m_pageCursor.y);
    m_pCurrentBox->set_width(m_availableWidth);
    m_pCurrentBox->set_height(m_availableHeight);
}

////////---------------------------------------------------------------------------------------
//////void DocLayouter::add_content_to_main_sizer()
//////{
//////    ImoDocument* pDoc = get_document();
//////    ImoContent* pContent = pDoc->get_content();
//////    int numItems = pContent->get_num_items();
//////    for (int i=0; i < numItems; i++)
//////    {
//////        ImoDocObj* pItem = pContent->get_item(i);
//////        GmoBox* pBox = m_pGModel->create_main_box_for(pItem);
//////        m_pMainSizer->add_child( new SizerChild(pBox) );
//////    }
//////}
//////
////////---------------------------------------------------------------------------------------
//////void DocLayouter::assign_space_to_content_items()
//////{
//////    ImoDocument* pDoc = get_document();
//////    m_pMainSizer->layout(pDoc->get_paper_width(), pDoc->get_paper_height());
//////}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_content()
{
    ImoDocument* pDoc = get_document();
    ImoContent* pContent = pDoc->get_content();
    int numItems = pContent->get_num_items();
    for (int i=0; i < numItems; i++)
    {
        ImoDocObj* pItem = pContent->get_item(i);
        layout_item(m_pCurrentBox, pItem);
    }
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_item(GmoBox* pParentBox, ImoDocObj* pItem)
{
    //for unit tests I need to save last used layouter. 
    delete m_pCurLayouter;
    m_pCurLayouter = new_item_layouter(pItem);
    m_pCurLayouter->prepare_to_start_layout();
    while (!m_pCurLayouter->is_item_layouted())
    {
        GmoBox* pPageBox = create_item_pagebox(pParentBox, m_pCurLayouter);
        m_pCurLayouter->layout_in_page(pPageBox);
        //prepare_next_document_page_if_needed();
    }
}

//---------------------------------------------------------------------------------------
GmoBox* DocLayouter::create_item_pagebox(GmoBox* pParentBox, ContentLayouter* pLayouter)
{
     //add new page box for item and assign space to it:
     //   width: as required by sizer policy
     //   height: all remaining height in current page.

    GmoBox* pPageBox = pLayouter->create_pagebox(pParentBox);
    pPageBox->set_width( pParentBox->get_width() );
    pPageBox->set_height( pParentBox->get_height() );
    return pPageBox;
}

//---------------------------------------------------------------------------------------
ContentLayouter* DocLayouter::new_item_layouter(ImoDocObj* pImo)
{
    //factory method

    switch (pImo->get_obj_type())
    {
        case ImoObj::k_score:   return new ScoreLayouter(pImo, m_pGModel, m_libraryScope);
        default:
            return new NullLayouter(pImo, m_pGModel);
    }
}

//---------------------------------------------------------------------------------------
ImoDocument* DocLayouter::get_document()
{
    return dynamic_cast<ImoDocument*>( m_pIModel->get_root() );
}



}  //namespace lomse
