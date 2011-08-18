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

#include "lomse_content_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_document_layouter.h"
#include "lomse_sizers.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// ContentLayouter implementation
//=======================================================================================
ContentLayouter::ContentLayouter(ImoContentObj* pItem, Layouter* pParent,
                                 GraphicModel* pGModel, LibraryScope& libraryScope,
                                 ImoStyles* pStyles)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles)
    , m_pContent( dynamic_cast<ImoContent*>(pItem) )
{
}

//---------------------------------------------------------------------------------------
ContentLayouter::~ContentLayouter()
{
}

//---------------------------------------------------------------------------------------
void ContentLayouter::layout_in_box()
{
    set_cursor_and_available_space(m_pItemMainBox);

    TreeNode<ImoObj>::children_iterator it;
    for (it = m_pContent->begin(); it != m_pContent->end(); ++it)
    {
        layout_item( dynamic_cast<ImoContentObj*>( *it ), m_pItemMainBox );
    }
    set_layout_is_finished(true);
}

//---------------------------------------------------------------------------------------
void ContentLayouter::create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                                      LUnits height)
{
    m_pItemMainBox = new GmoBoxDocPageContent(m_pContent);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}


}  //namespace lomse
