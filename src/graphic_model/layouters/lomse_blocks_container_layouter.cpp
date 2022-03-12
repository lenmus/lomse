//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_blocks_container_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_document_layouter.h"
#include "lomse_sizers.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"
#include "lomse_inlines_container_layouter.h"
#include "lomse_score_player_ctrl.h"
#include "lomse_logger.h"


namespace lomse
{

//=======================================================================================
// ContentLayouter implementation
//=======================================================================================
ContentLayouter::ContentLayouter(ImoContentObj* pItem, Layouter* pParent,
                                 GraphicModel* pGModel, LibraryScope& libraryScope,
                                 ImoStyles* pStyles, bool fAddShapesToModel)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles, fAddShapesToModel)
    , m_pContent( dynamic_cast<ImoContent*>(pItem) )
{
}

//---------------------------------------------------------------------------------------
void ContentLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    set_cursor_and_available_space();

    TreeNode<ImoObj>::children_iterator it;
    int result = k_layout_success;
    for (it = m_pContent->begin(); it != m_pContent->end(); ++it)
    {
        result = layout_item(static_cast<ImoContentObj*>( *it ), m_pItemMainBox, m_constrains);
        if (result == k_layout_failed_auto_scale)
            break;
    }
    set_layout_result(result);
}

//---------------------------------------------------------------------------------------
void ContentLayouter::create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                                      LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxDocPageContent(m_pContent);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}


//=======================================================================================
// MultiColumnLayouter implementation
//=======================================================================================
MultiColumnLayouter::MultiColumnLayouter(ImoContentObj* pItem, Layouter* pParent,
                                 GraphicModel* pGModel, LibraryScope& libraryScope,
                                 ImoStyles* pStyles, bool fAddShapesToModel)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles, fAddShapesToModel)
    , m_pMultiColumn( dynamic_cast<ImoMultiColumn*>(pItem) )
{
}

//---------------------------------------------------------------------------------------
MultiColumnLayouter::~MultiColumnLayouter()
{
    vector<Layouter*>::iterator it;
    for (it = m_colLayouters.begin(); it != m_colLayouters.end(); ++it)
        delete *it;
    m_colLayouters.clear();
}

//---------------------------------------------------------------------------------------
void MultiColumnLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    set_cursor_and_available_space();

    //get total witdh
    LUnits totalWidth = m_pItemMainBox->get_content_width();
    UPoint curPos(m_pItemMainBox->get_content_left(), m_pItemMainBox->get_content_top());

    //create column layouters and assign position and width to each column
    ImoContent* pWrapper = m_pMultiColumn->get_content();
    TreeNode<ImoObj>::children_iterator it;
    int iCol;
    for (iCol=0, it = pWrapper->begin(); it != pWrapper->end(); ++it, ++iCol)
    {
        //create column layouter and prepare it to layout the column
        ImoContent* pContent = static_cast<ImoContent*>( *it );
        m_colLayouters.push_back( create_layouter(pContent) );
        Layouter* pCurLayouter = m_colLayouters.back();
        pCurLayouter->prepare_to_start_layout();

        //assign width and position to this column
        LUnits width = (totalWidth * m_pMultiColumn->get_column_width(iCol)) / 100.0f;
        m_colWidth.push_back(width);
        m_colPosition.push_back(curPos);
        curPos.x += width;
    }

    //loop to layout columns while at least one column layout not finished
    int numCols = m_pMultiColumn->get_num_columns();
    bool fLayoutFinished = false;
    int layoutResult = k_layout_not_finished;
    while(!fLayoutFinished)
    {
        fLayoutFinished = true;        //assume no pagebreaks needed
        for (iCol=0; iCol < numCols; ++iCol)
        {
            Layouter* pCurLayouter = m_colLayouters[iCol];
            //layout this column if not yet finished
            if (!pCurLayouter->is_item_layouted())
            {
                layout_column(pCurLayouter, m_pItemMainBox, m_colWidth[iCol],
                              m_colPosition[iCol]);
                fLayoutFinished &= pCurLayouter->is_item_layouted();
                layoutResult = pCurLayouter->get_layout_result();
            }
        }

        //start new page if layout not finished
        if (!fLayoutFinished)
        {
            m_pItemMainBox = start_new_page();
            for (int i=0; i < numCols; ++i)
                m_colPosition[i].y = m_pItemMainBox->get_content_top();
        }
    }

    //loop to finish columns
    if (layoutResult != k_layout_failed_auto_scale)
    {
        LUnits bottom = 0.0f;
        LUnits height = 0.0f;
        for (iCol=0; iCol < numCols; ++iCol)
        {
            Layouter* pCurLayouter = m_colLayouters[iCol];
            pCurLayouter->add_end_margins();

            GmoBox* pChildBox = pCurLayouter->get_item_main_box();
            bottom = max(bottom, pChildBox->get_bottom());
            height = max(height, pChildBox->get_height());
        }

        //update cursor and available space
        m_pageCursor.y = bottom;
        m_availableHeight -= height;
    }
    set_layout_result(layoutResult);
}

//---------------------------------------------------------------------------------------
void MultiColumnLayouter::create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                                          LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxDocPageContent(m_pMultiColumn);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
void MultiColumnLayouter::layout_column(Layouter* pColLayouter, GmoBox* pParentBox,
                                        LUnits width, UPoint pos)
{
    pColLayouter->create_main_box(pParentBox, pos, width, m_availableHeight);
    pColLayouter->layout_in_box();
    pColLayouter->set_box_height();
}


//=======================================================================================
// ListLayouter implementation
//=======================================================================================
ListLayouter::ListLayouter(ImoContentObj* pItem, Layouter* pParent,
                           GraphicModel* pGModel, LibraryScope& libraryScope,
                           ImoStyles* pStyles, bool fAddShapesToModel)
    : BlocksContainerLayouter(pItem, pParent, pGModel, libraryScope, pStyles,
                              fAddShapesToModel)
    , m_indent(1000.0f)        //1 cm
{
}

//---------------------------------------------------------------------------------------
void ListLayouter::create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                                   LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxDocPageContent(m_pItem);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos.x + m_indent, pos.y);
    m_pItemMainBox->set_width(width - m_indent);
    m_pItemMainBox->set_height(height);
}



//=======================================================================================
// BlocksContainerLayouter implementation
//=======================================================================================
BlocksContainerLayouter::BlocksContainerLayouter(ImoContentObj* pImo, Layouter* pParent,
                                   GraphicModel* pGModel, LibraryScope& libraryScope,
                                   ImoStyles* pStyles, bool fAddShapesToModel)
    : Layouter(pImo, pParent, pGModel, libraryScope, pStyles, fAddShapesToModel)
{
}

//---------------------------------------------------------------------------------------
void BlocksContainerLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    set_cursor_and_available_space();

    ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pItem);
    ImoContent* pContent = pBlock->get_content();

    TreeNode<ImoObj>::children_iterator it;
    for (it = pContent->begin(); it != pContent->end(); ++it)
    {
        ImoContentObj* pObj = dynamic_cast<ImoContentObj*>( *it );
        if (pObj)
            layout_item(pObj, m_pItemMainBox, m_constrains);
        else
            LOMSE_LOG_ERROR("Invalid IMO tree. Child of ImoBlocksContainer "
                            "is not ImoContentObj");
    }
    set_layout_result(k_layout_success);
}

//---------------------------------------------------------------------------------------
void BlocksContainerLayouter::create_main_box(GmoBox* pParentBox, UPoint pos,
                                              LUnits width, LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxDocPageContent(m_pItem);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos.x, pos.y);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}



//=======================================================================================
// ListItemLayouter implementation
//=======================================================================================
ListItemLayouter::ListItemLayouter(ImoContentObj* pImo, Layouter* pParent,
                                   GraphicModel* pGModel, LibraryScope& libraryScope,
                                   ImoStyles* pStyles, bool fAddShapesToModel)
    : BlocksContainerLayouter(pImo, pParent, pGModel, libraryScope, pStyles,
                              fAddShapesToModel)
{
}

//---------------------------------------------------------------------------------------
bool ListItemLayouter::is_first_content_item(ImoContentObj* pImo)
{
    ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pItem);
    ImoContent* pContent = pBlock->get_content();
    return (pImo == pContent->get_first_content_item());
}



}  //namespace lomse
