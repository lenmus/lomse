//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_document_layouter.h"

#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "private/lomse_document_p.h"
#include "lomse_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_calligrapher.h"
#include "lomse_box_system.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// DocLayouter implementation
//  Main object responsible for laying out a document. It takes care of creating pages,
//  adding footers and headers, controlling of margins, available space, page
//  numbering, etc. And delegates content layout to ContentLayouter object who, in turn,
//  delegates in specialized layouters.
//---------------------------------------------------------------------------------------

DocLayouter::DocLayouter(Document* pDoc, LibraryScope& libraryScope, int constrains,
                         LUnits width)
    : Layouter(libraryScope)
    , m_pDoc( pDoc->get_im_root() )
    , m_viewWidth(width)
    , m_pScoreLayouter(nullptr)
{
    m_pStyles = m_pDoc->get_styles();
    m_pGModel = LOMSE_NEW GraphicModel(m_pDoc);
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
    int result = k_layout_not_finished;
    int numTrials = 0;
    while(result == k_layout_not_finished && numTrials < 30)
    {
        numTrials++;
        start_new_page();
        result = layout_content();
        if (result == k_layout_failed_auto_scale)
        {
            delete_last_trial();
            result = k_layout_not_finished;
        }
    }
    if (result == k_layout_not_finished)
        layout_empty_document();
    else
        fix_document_size();
}

//---------------------------------------------------------------------------------------
void DocLayouter::delete_last_trial()
{
    delete m_pScoreLayouter;
    delete m_pGModel;

    m_result = k_layout_not_finished;
    m_pGModel = LOMSE_NEW GraphicModel(m_pDoc);
    m_pParentLayouter = nullptr;
    m_pStyles = nullptr;
    m_pItemMainBox = nullptr;
    m_pCurLayouter = nullptr;
    m_pItem = nullptr;
    m_fAddShapesToModel = false;
    m_constrains = 0;
    m_availableWidth = 0.0f;
    m_availableHeight = 0.0f;
    m_pScoreLayouter = nullptr;
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
    //width
    if (m_constrains & k_infinite_width)
        m_availableWidth = LOMSE_INFINITE_LENGTH;
    else if (m_constrains & k_use_viewport_width)
    {
        m_availableWidth = m_viewWidth;
    }
    else
        m_availableWidth = m_pDoc->get_paper_width() / m_pDoc->get_page_content_scale();

    //height
    m_availableHeight = (m_constrains & k_infinite_height) ? LOMSE_INFINITE_LENGTH
                         : m_pDoc->get_paper_height() / m_pDoc->get_page_content_scale();

    pBox->set_width(m_availableWidth);
    pBox->set_height(m_availableHeight);
}

//---------------------------------------------------------------------------------------
void DocLayouter::add_margins_to_page(GmoBoxDocPage* pPage)
{
    ImoPageInfo* pInfo = m_pDoc->get_page_info();
    LUnits top = 0.0f;
    LUnits bottom = 0.0f;
    LUnits left = 0.0f;
    LUnits right = 0.0f;
    if (pPage->get_number() % 2 == 0)
    {
        top = pInfo->get_top_margin_even() / m_pDoc->get_page_content_scale();
        bottom = pInfo->get_bottom_margin_even() / m_pDoc->get_page_content_scale();
        left = pInfo->get_left_margin_even() / m_pDoc->get_page_content_scale();
        right = pInfo->get_right_margin_even() / m_pDoc->get_page_content_scale();
    }
    else
    {
        top = pInfo->get_top_margin_odd() / m_pDoc->get_page_content_scale();
        bottom = pInfo->get_bottom_margin_odd() / m_pDoc->get_page_content_scale();
        left = pInfo->get_left_margin_odd() / m_pDoc->get_page_content_scale();
        right = pInfo->get_right_margin_odd() / m_pDoc->get_page_content_scale();
    }

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
            if (pPage->get_number() % 2 == 0)
                height += pInfo->get_bottom_margin_even() / m_pDoc->get_page_content_scale();
            else
                height += pInfo->get_bottom_margin_odd() / m_pDoc->get_page_content_scale();
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
        //free flow view, single page view
        //height determined by BoxDocPageContent
        GmoBoxDocPage* pPage = static_cast<GmoBoxDocPage*>(m_pItemMainBox);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        LUnits height = pBDPC->get_size().height + 2.0f * pBDPC->get_origin().y;
        pPage->set_height(height);
    }
}

//---------------------------------------------------------------------------------------
int DocLayouter::layout_content()
{
    return layout_item(m_pDoc->get_content(), m_pItemMainBox, m_constrains);
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
