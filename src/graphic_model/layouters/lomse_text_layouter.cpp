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

#include "lomse_text_layouter.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ParagraphLayouter implementation
//---------------------------------------------------------------------------------------

ParagraphLayouter::ParagraphLayouter(ImoDocObj* pImo, GraphicModel* pGModel,
                                     LibraryScope& libraryScope)
    : ContentLayouter(pImo, pGModel)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoParagraph*>(pImo) )
{
}

//---------------------------------------------------------------------------------------
ParagraphLayouter::~ParagraphLayouter()
{
}


////---------------------------------------------------------------------------------------
//void ParagraphLayouter::prepare_to_start_layout()
//{
//    ContentLayouter::prepare_to_start_layout();
//
//    create_stub_for_score();
//    create_instrument_engravers();
//    get_score_renderization_options();
//    decide_systems_indentation();
//
//    m_pColsBuilder->trace_column(m_iColumnToTrace);
//    m_pColsBuilder->create_columns();
//
//	m_iCurPage = -1;
//    m_iCurColumn = -1;
//    m_iCurSystem = -1;
//}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::layout_in_page(GmoBox* pContainerBox)
{
    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_page() method must always continue layouting from current state.

    pContainerBox->set_height(1000.0f);      //test
    //page_initializations(pContainerBox);

    //move_cursor_to_top_left_corner();
    //if (is_first_page())
    //{
    //    decide_line_breaks();
    //    add_score_titles();
    //    move_cursor_after_headers();
    //}

    //while(enough_space_in_page() )//&& m_iCurItem < get_num_items())
    {
        add_text_item();
    }

    bool fMoreText = false; //m_iCurItem < get_num_items();
    set_layout_is_finished( !fMoreText );

    ////if layout finished adjust height of last page (remove unused space)
    //if (!fMoreText)
    //{
    //    LUnits yBottom = m_pCurBoxSystem->get_bottom();
    //    m_pCurBoxPage->set_height( yBottom - m_pCurBoxPage->get_top());
    //}
}

//---------------------------------------------------------------------------------------
GmoBox* ParagraphLayouter::create_main_box()
{
    return new GmoBoxParagraph();
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_text_item()
{
}



}  //namespace lomse
