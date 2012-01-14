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

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_calligrapher.h"

////remove warning for using 'this' in constructor
//#if (LOMSE_COMPILER_MSVC == 1)
//#pragma warning(disable:4355)
//#endif

namespace lomse
{

//---------------------------------------------------------------------------------------
// DocLayouter implementation
//  Main object responsible for layouting a document. It takes care of creating pages,
//  adding footers and headers, controlling of margins, available space, page
//  numbering, etc. And delegates content layout to ContentLayouter object who, in turn,
//  delegates in specialized layouters.
//---------------------------------------------------------------------------------------

DocLayouter::DocLayouter(InternalModel* pIModel, LibraryScope& libraryScope)
    : Layouter(libraryScope)
    , m_pScoreLayouter(NULL)
{
    m_pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
    m_pStyles = m_pDoc->get_styles();
    m_pGModel = LOMSE_NEW GraphicModel();
}

//---------------------------------------------------------------------------------------
DocLayouter::~DocLayouter()
{
    delete m_pScoreLayouter;
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_document()
{
    start_new_page();
    layout_content();
    m_pGModel->set_ready(true);
}

//---------------------------------------------------------------------------------------
GmoBox* DocLayouter::start_new_page()
{
    GmoBoxDocPage* pPage = create_document_page();
    assign_paper_size_to(pPage);
    add_margins_to_page(pPage);
    add_headers_to_page(pPage);
    add_footers_to_page(pPage);

    m_pItemMainBox = pPage;
    return pPage;
}

//---------------------------------------------------------------------------------------
GmoBoxDocPage* DocLayouter::create_document_page()
{
    GmoBoxDocument* pBoxDoc = m_pGModel->get_root();
    GmoBoxDocPage* pPage = pBoxDoc->add_new_page();
    return pPage;
}

//---------------------------------------------------------------------------------------
void DocLayouter::assign_paper_size_to(GmoBox* pBox)
{
    m_availableWidth = m_pDoc->get_paper_width();
    m_availableHeight = m_pDoc->get_paper_height();

    pBox->set_width(m_availableWidth);
    pBox->set_height(m_availableHeight);
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_margins_to_page(GmoBoxDocPage* pPage)
{
    ImoPageInfo* pInfo = m_pDoc->get_page_info();
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
    m_availableWidth -= (left + right);
    m_availableHeight -= (top + bottom);
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
void DocLayouter::layout_content()
{
    layout_item( m_pDoc->get_content(), m_pItemMainBox );
}

//---------------------------------------------------------------------------------------
void DocLayouter::save_score_layouter(Layouter* pLayouter)
{
    delete m_pScoreLayouter;
    m_pScoreLayouter = pLayouter;
}

//---------------------------------------------------------------------------------------
ScoreLayouter* DocLayouter::get_score_layouter()
{
    return dynamic_cast<ScoreLayouter*>( m_pScoreLayouter );
}


}  //namespace lomse
