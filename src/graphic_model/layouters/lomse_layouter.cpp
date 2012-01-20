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

#include "lomse_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_content_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_box_content_layouter.h"

namespace lomse
{

//=======================================================================================
// Layouter implementation
//=======================================================================================
Layouter::Layouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
                   LibraryScope& libraryScope, ImoStyles* pStyles)
    : m_fIsLayouted(false)
    , m_pGModel(pGModel)
    , m_pParentLayouter(pParent)
    , m_libraryScope(libraryScope)
    , m_pStyles(pStyles)
    , m_pItemMainBox(NULL)
    , m_pItem(pItem)
{
}

//---------------------------------------------------------------------------------------
// constructor for DocumentLayouter
Layouter::Layouter(LibraryScope& libraryScope)
    : m_fIsLayouted(false)
    , m_pGModel(NULL)
    , m_pParentLayouter(NULL)
    , m_libraryScope(libraryScope)
    , m_pStyles(NULL)
    , m_pItemMainBox(NULL)
    , m_pItem(NULL)
{
}

//---------------------------------------------------------------------------------------
GmoBox* Layouter::start_new_page()
{
    GmoBox* pParentBox = m_pParentLayouter->start_new_page();

    m_pageCursor = m_pParentLayouter->get_cursor();
    m_availableWidth = m_pParentLayouter->get_available_width();
    m_availableHeight = m_pParentLayouter->get_available_height();

    create_main_box(pParentBox, m_pageCursor, m_availableWidth, m_availableHeight);

    return m_pItemMainBox;
}

//---------------------------------------------------------------------------------------
void Layouter::layout_item(ImoContentObj* pItem, GmoBox* pParentBox)
{
    m_pCurLayouter = create_layouter(pItem);

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
    m_pageCursor.y = pChildBox->get_bottom();
    m_availableHeight -= pChildBox->get_height();

    if (!pItem->is_score())
        delete m_pCurLayouter;
}

//---------------------------------------------------------------------------------------
void Layouter::set_cursor_and_available_space(GmoBox* pItemMainBox)
{
    m_pageCursor.x = m_pItemMainBox->get_content_left();
    m_pageCursor.y = m_pItemMainBox->get_content_top();

    m_availableWidth = m_pItemMainBox->get_content_width();
    m_availableHeight = m_pItemMainBox->get_content_height();
}

//---------------------------------------------------------------------------------------
void Layouter::set_box_height()
{
    LUnits start = m_pItemMainBox->get_origin().y;
    m_pItemMainBox->set_height( m_pageCursor.y - start );
}

//---------------------------------------------------------------------------------------
void Layouter::add_end_margins()
{
    ImoStyle* pStyle = m_pItem->get_style();
    if (pStyle)
    {
        LUnits space = pStyle->get_lunits_property(ImoStyle::k_margin_bottom)
                       + pStyle->get_lunits_property(ImoStyle::k_border_width_bottom)
                       + pStyle->get_lunits_property(ImoStyle::k_padding_bottom);
        m_pItemMainBox->set_height( m_pItemMainBox->get_height() + space );
        m_pageCursor.y += space;
    }
}

//---------------------------------------------------------------------------------------
Layouter* Layouter::create_layouter(ImoContentObj* pItem)
{
    Layouter* pLayouter = LayouterFactory::create_layouter(pItem, this, m_pStyles);
    if (pItem->is_score())
        save_score_layouter(pLayouter);
    return pLayouter;
}


//=======================================================================================
// LayouterFactory implementation
//=======================================================================================
Layouter* LayouterFactory::create_layouter(ImoContentObj* pItem, Layouter* pParent,
                                           ImoStyles* pStyles)
{
    GraphicModel* pGModel = pParent->get_graphic_model();
    LibraryScope& libraryScope = pParent->get_library_scope();

    switch (pItem->get_obj_type())
    {
//        case k_imo_control:
//            return LOMSE_NEW ControlLayouter(pItem, pParent, pGModel, libraryScope, pStyles);

        case k_imo_dynamic:
        case k_imo_content:
            return LOMSE_NEW ContentLayouter(pItem, pParent, pGModel, libraryScope, pStyles);

        case k_imo_list:
            return LOMSE_NEW ListLayouter(pItem, pParent, pGModel, libraryScope, pStyles);

        case k_imo_para:
        case k_imo_heading:
        case k_imo_listitem:
            return LOMSE_NEW BoxContentLayouter(pItem, pParent, pGModel, libraryScope, pStyles);

        case k_imo_multicolumn:
            return LOMSE_NEW MultiColumnLayouter(pItem, pParent, pGModel, libraryScope, pStyles);

        case k_imo_score:
            return LOMSE_NEW ScoreLayouter(pItem, pParent, pGModel, libraryScope);

        default:
            return LOMSE_NEW NullLayouter(pItem, pParent, pGModel, libraryScope);
    }
}


}  //namespace lomse
