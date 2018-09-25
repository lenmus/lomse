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

#include "lomse_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_blocks_container_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_inlines_container_layouter.h"
#include "lomse_table_layouter.h"
#include "lomse_logger.h"

namespace lomse
{

//=======================================================================================
// Layouter implementation
//=======================================================================================
Layouter::Layouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
                   LibraryScope& libraryScope, ImoStyles* pStyles,
                   bool fAddShapesToModel)
    : m_fIsLayouted(false)
    , m_pGModel(pGModel)
    , m_pParentLayouter(pParent)
    , m_libraryScope(libraryScope)
    , m_pStyles(pStyles)
    , m_pItemMainBox(nullptr)
    , m_pCurLayouter(nullptr)
    , m_pItem(pItem)
    , m_fAddShapesToModel(fAddShapesToModel)
    , m_constrains(0)
    , m_availableWidth(0.0f)
    , m_availableHeight(0.0f)
{
}

//---------------------------------------------------------------------------------------
// constructor for DocumentLayouter
Layouter::Layouter(LibraryScope& libraryScope)
    : m_fIsLayouted(false)
    , m_pGModel(nullptr)
    , m_pParentLayouter(nullptr)
    , m_libraryScope(libraryScope)
    , m_pStyles(nullptr)
    , m_pItemMainBox(nullptr)
    , m_pCurLayouter(nullptr)
    , m_pItem(nullptr)
    , m_fAddShapesToModel(false)
    , m_constrains(0)
    , m_availableWidth(0.0f)
    , m_availableHeight(0.0f)
{
}

//---------------------------------------------------------------------------------------
GmoBox* Layouter::start_new_page()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, string(""));

    GmoBox* pParentBox = m_pParentLayouter->start_new_page();

    m_pageCursor = m_pParentLayouter->get_cursor();
    m_availableWidth = m_pParentLayouter->get_available_width();
    m_availableHeight = m_pParentLayouter->get_available_height();

    create_main_box(pParentBox, m_pageCursor, m_availableWidth, m_availableHeight);

    return m_pItemMainBox;
}

//---------------------------------------------------------------------------------------
void Layouter::layout_item(ImoContentObj* pItem, GmoBox* pParentBox, int constrains)
{
    LOMSE_LOG_DEBUG(Logger::k_layout,
        "Laying out id %d %s", pItem->get_id(), pItem->get_name().c_str());

    m_pCurLayouter = create_layouter(pItem);
    m_pCurLayouter->set_constrains(constrains);

    m_pCurLayouter->prepare_to_start_layout();
    while (!m_pCurLayouter->is_item_layouted())
    {
        m_pCurLayouter->create_main_box(pParentBox, m_pageCursor,
                                        m_availableWidth, m_availableHeight);
        m_pCurLayouter->layout_in_box();
        m_pCurLayouter->set_box_height();

        if (!m_pCurLayouter->is_item_layouted())
        {
            pParentBox = start_new_page();
        }
    }
    m_pCurLayouter->add_end_margins();

    //update cursor and available space
    GmoBox* pChildBox = m_pCurLayouter->get_item_main_box();
    if (pChildBox)  //AWARE: NullLayouter does not create a box
    {
        m_pageCursor.y = pChildBox->get_bottom();
        m_availableHeight -= pChildBox->get_height();
    }

    if (!pItem->is_score())
        delete m_pCurLayouter;
}

//---------------------------------------------------------------------------------------
void Layouter::set_cursor_and_available_space()
{
    m_pageCursor.x = m_pItemMainBox->get_content_left();
    m_pageCursor.y = m_pItemMainBox->get_content_top();

    m_availableWidth = m_pItemMainBox->get_content_width();
    m_availableHeight = m_pItemMainBox->get_content_height();

    LOMSE_LOG_DEBUG(Logger::k_layout,
        "cursor at(%.2f, %.2f), available space(%.2f, %.2f)",
        m_pageCursor.x, m_pageCursor.y, m_availableWidth, m_availableHeight);
}

//---------------------------------------------------------------------------------------
LUnits Layouter::set_box_height()
{
    LUnits start = m_pItemMainBox->get_origin().y;
    LUnits height = m_pageCursor.y - start;
    ImoStyle* pStyle = m_pItem->get_style();
    if (pStyle)
        height = max(pStyle->min_height(), height);
    m_pItemMainBox->set_height(height);
    return height;
}

//---------------------------------------------------------------------------------------
void Layouter::add_end_margins()
{
    ImoStyle* pStyle = m_pItem->get_style();
    if (pStyle && m_pItemMainBox)   //NullLayouter doesn't have main box
    {
        LUnits space = pStyle->margin_bottom()
                       + pStyle->border_width_bottom()
                       + pStyle->padding_bottom();
        m_pItemMainBox->set_height( m_pItemMainBox->get_height() + space );
        m_pageCursor.y += space;
    }
}

//---------------------------------------------------------------------------------------
Layouter* Layouter::create_layouter(ImoContentObj* pItem, int constrains)
{
    Layouter* pLayouter = LayouterFactory::create_layouter(pItem, this);
    if (pItem->is_score())
    {
        save_score_layouter(pLayouter);
        pLayouter->set_constrains(constrains);
    }
    return pLayouter;
}




//=======================================================================================
// LayouterFactory implementation
//=======================================================================================
Layouter* LayouterFactory::create_layouter(ImoContentObj* pItem, Layouter* pParent)
{
    GraphicModel* pGModel = pParent->get_graphic_model();
    LibraryScope& libraryScope = pParent->get_library_scope();
    ImoStyles* pStyles = pParent->get_styles();
    bool fAddShapesToModel
        = compute_value_for_add_shapes_flag(pItem, pParent->must_add_shapes_to_model());

    switch (pItem->get_obj_type())
    {
        //blocks container objects
        case k_imo_dynamic:
        case k_imo_content:
            return LOMSE_NEW ContentLayouter(pItem, pParent, pGModel, libraryScope,
                                             pStyles, fAddShapesToModel);

        case k_imo_multicolumn:
            return LOMSE_NEW MultiColumnLayouter(pItem, pParent, pGModel, libraryScope,
                                                 pStyles, fAddShapesToModel);

        case k_imo_list:
            return LOMSE_NEW ListLayouter(pItem, pParent, pGModel, libraryScope,
                                          pStyles, fAddShapesToModel);

        case k_imo_listitem:
            return LOMSE_NEW ListItemLayouter(pItem, pParent, pGModel, libraryScope,
                                              pStyles, fAddShapesToModel);

        case k_imo_table_cell:
            return LOMSE_NEW TableCellLayouter(pItem, pParent, pGModel, libraryScope,
                                               pStyles);    //never adds shapes, until row ready

        case k_imo_score:
            return LOMSE_NEW ScoreLayouter(pItem, pParent, pGModel, libraryScope);

        case k_imo_table:
            return LOMSE_NEW TableLayouter(pItem, pParent, pGModel, libraryScope,
                                           pStyles, fAddShapesToModel);

        // inlines container objects
        case k_imo_anonymous_block:
        case k_imo_para:
        case k_imo_heading:
            return LOMSE_NEW InlinesContainerLayouter(pItem, pParent, pGModel, libraryScope,
                                                      pStyles, fAddShapesToModel);

        default:
            return LOMSE_NEW NullLayouter(pItem, pParent, pGModel, libraryScope);
    }
}

//---------------------------------------------------------------------------------------
bool LayouterFactory::compute_value_for_add_shapes_flag(ImoContentObj* pItem,
                                                        bool fInheritedValue)
{
    //children of main content block always add shapes
    ImoObj* pParent = pItem->get_parent_imo();
    if (pParent->is_content() && pParent->get_parent_imo()->is_document())
        return true;

    //objects inside a table cell never add shapes. Table cell shapes wiil
    //be added when the table row is fully laid out.
    if (pParent->is_table_cell())
        return false;

    //otherwise inherit from parent
        return fInheritedValue;
}


}  //namespace lomse
