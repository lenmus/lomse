//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_document_layouter.h"

#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_document.h"
#include "lomse_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// DocLayouter implementation
//  Main object responsible for laying out a document. It takes care of creating pages,
//  adding footers and headers, controlling of margins, available space, page
//  numbering, etc. And delegates content layout to ContentLayouter object who, in turn,
//  delegates in specialized layouters.
//---------------------------------------------------------------------------------------

DocLayouter::DocLayouter(Document* pDoc, LibraryScope& libraryScope, int constrains)
    : Layouter(libraryScope)
    , m_pScoreLayouter(nullptr)
{
    m_pDoc = pDoc->get_im_root();
    m_pStyles = m_pDoc->get_styles();
    m_pGModel = LOMSE_NEW GraphicModel();
    m_pageWidth = pDoc->get_paper_width();
    m_pageHeight = pDoc->get_paper_height();
    m_constrains = constrains;
}

//---------------------------------------------------------------------------------------
DocLayouter::~DocLayouter()
{
    delete m_pScoreLayouter;
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_empty_document()
{
    GmoBoxDocPage* pPage = create_document_page();
    m_pItemMainBox = pPage;
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_document()
{
    start_new_page();
    layout_content();
    fix_document_size();
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
    m_availableWidth = (m_constrains & k_infinite_width) ?
                            LOMSE_INFINITE_LENGTH : m_pDoc->get_paper_width();

    m_availableHeight = (m_constrains & k_infinite_height) ?
                            LOMSE_INFINITE_LENGTH : m_pDoc->get_paper_height();

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
void DocLayouter::add_headers_to_page(GmoBoxDocPage* UNUSED(pPage))
{
    //TODO: DocLayouter::add_headers_to_page
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_footers_to_page(GmoBoxDocPage* UNUSED(pPage))
{
    //TODO: DocLayouter::add_footers_to_page
}

//---------------------------------------------------------------------------------------
void DocLayouter::fix_document_size()
{
    if (m_constrains & k_infinite_width)
    {
        GmoBoxDocPage* pPage = static_cast<GmoBoxDocPage*>(m_pItemMainBox);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);     //System

        //View height and width are determined by BoxSystem.
        //To set page width it is necessary to fix width in BoxDocPage, BoxDocPageContent
        //and BoxScorePage.
        //To set page height it is only necessary to fix height in BoxDocPage.
        //BoxSystem does not exist when error "not enough space in page"
        if (pBSys)
        {
            LUnits width = pBSys->get_size().width + pBSys->get_origin().x;
            pBSys->set_width(width);
            pBSP->set_width(width);
            pBDPC->set_width(width);
            width += pBSP->get_origin().x;
            pPage->set_width(width);

            LUnits height = pBSys->get_size().height + pBSys->get_origin().y;
            ImoPageInfo* pInfo = m_pDoc->get_page_info();
            height += pInfo->get_bottom_margin();
            pPage->set_height(height);
        }
        else
        {
            //an arbitrary with
            LUnits width = 21000.0f;
            pBSP->set_width(width);
            pBDPC->set_width(width);
            width += pBSP->get_origin().x;
            pPage->set_width(width);

            //height determined by BoxDocPageContent
            LUnits height = pBDPC->get_size().height + 2.0f * pBDPC->get_origin().y;
            pPage->set_height(height);
        }
    }

    if (m_constrains & k_infinite_height)
    {
        //TODO: free flow view
    }
}

//---------------------------------------------------------------------------------------
void DocLayouter::layout_content()
{
    layout_item(m_pDoc->get_content(), m_pItemMainBox, m_constrains);
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
