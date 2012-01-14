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

#include "lomse_wrapper_box_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
//#include "lomse_text_engraver.h"
//#include "lomse_shape_text.h"
//#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// WrapperBoxLayouter implementation
//=======================================================================================
WrapperBoxLayouter::WrapperBoxLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                                       LibraryScope& libraryScope, ImoStyles* pStyles)
    : ContentLayouter(pImo, pGModel, NULL, NULL)    //pDoc, pDocLayouter)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoTextBlock*>(pImo) )
    , m_pStyles(pStyles)
    , m_pMainBox(NULL)
{
}

//---------------------------------------------------------------------------------------
WrapperBoxLayouter::~WrapperBoxLayouter()
{
}

////---------------------------------------------------------------------------------------
//void WrapperBoxLayouter::prepare_to_start_layout()
//{
//}

////---------------------------------------------------------------------------------------
//void WrapperBoxLayouter::layout_in_page(GmoBox* pMainBox)
//{
//    ////AWARE: This method is invoked to layout a page. If there are more pages to
//    ////layout, it will be invoked more times. Therefore, this method must not initialize
//    ////anything. All initializations must be done in 'prepare_to_start_layout()'.
//    ////layout_in_page() method must always continue layouting from current state.
//
//    //page_initializations(pMainBox);
//
//    //while(enough_space_in_box() && more_cells())
//    //{
//    //    add_line();
//    //}
//
//    //bool fMoreText = more_cells();
//    //if (!fMoreText)
//    //{
//    //    //add bottom margin
//    //    ImoStyle* pStyle = m_pPara->get_style();
//    //    // 1pt = 1/72" = 25.4/72 mm = 2540/72 LU = 35.2777777777
//    //    LUnits margin = pStyle->margin_bottom() * pStyle->font_size() * 2540.0f / 72.0f;
//    //    LUnits height = pMainBox->get_height();
//    //    pMainBox->set_height( height + margin );
//    //    m_cursor.y += margin;
//
//    //    //copy shapes in BoxDocPage
//    //    pMainBox->store_shapes_in_doc_page();
//    //}
//    //set_layout_is_finished( !fMoreText );
//}

////---------------------------------------------------------------------------------------
//GmoBox* WrapperBoxLayouter::create_main_box()
//{
//    m_pMainBox = new GmoBoxParagraph();
//    return m_pMainBox;
//}


}  //namespace lomse
