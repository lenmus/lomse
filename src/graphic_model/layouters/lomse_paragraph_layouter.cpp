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

#include "lomse_paragraph_layouter.h"
#include "lomse_build_options.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_shapes.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// ParagraphLayouter implementation
//=======================================================================================
ParagraphLayouter::ParagraphLayouter(ImoContentObj* pItem, Layouter* pParent,
                                     GraphicModel* pGModel, LibraryScope& libraryScope,
                                     ImoStyles* pStyles)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoTextBlock*>(pItem) )
{
}

//---------------------------------------------------------------------------------------
ParagraphLayouter::~ParagraphLayouter()
{
    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
        delete *it;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::prepare_to_start_layout()
{
    Layouter::prepare_to_start_layout();
    create_cells();
    point_to_first_cell();
    initialize_lines();
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::layout_in_box()
{
    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_box() method must always continue layouting from current state.

    set_cursor_and_available_space(m_pItemMainBox);
    page_initializations(m_pItemMainBox);

    if (!is_line_ready())
        prepare_line();

    while(is_line_ready() && enough_space_in_box())
    {
        add_line();
        prepare_line();
    }

    bool fMoreText = is_line_ready();
    if (!fMoreText)
        m_pItemMainBox->store_shapes_in_doc_page();

    set_layout_is_finished( !fMoreText );
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::create_main_box(GmoBox* pParentBox, UPoint pos,
                                        LUnits width, LUnits height)
{
    m_pItemMainBox = new GmoBoxParagraph(m_pPara);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::create_cells()
{
    CellsCreator creator(m_cells, m_libraryScope);
    creator.create_cells(m_pPara);
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::prepare_line()
{
    //After execution:
    //  m_itStart - will point to first cell to include or m_cells.end() if no more cells
    //  m_itEnd - will point to after last cell to include
    //  m_lineHeight - will contain line height

    m_itStart = more_cells() ? m_itCells :  m_cells.end();
    m_itEnd = m_cells.end();
    m_lineHeight = 0.0f;
    UPoint pos = m_pageCursor;

    while(more_cells() && space_in_line())
    {
        LUnits cellHeight = add_cell_to_line();
        m_lineHeight = max(m_lineHeight, cellHeight);
        next_cell();
    }
    m_itEnd = m_itCells;

    m_pageCursor = pos;     //restore cursor
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_line()
{
    LUnits left = m_pageCursor.x;       //save left margin
    std::list<Cell*>::iterator it;
    for(it = m_itStart; it != m_itEnd; ++it)
    {
        add_cell_shape(*it, m_lineHeight);
    }

    m_lineHeight *= 1.5f;     //TODO: now interline space is fixed: 0.5 em

    //prepare for next line
    m_pageCursor.x = left;
    m_pageCursor.y += m_lineHeight;
    m_availableSpace = m_availableWidth;

    m_pItemMainBox->set_height( m_pItemMainBox->get_height() + m_lineHeight );
}

//---------------------------------------------------------------------------------------
LUnits ParagraphLayouter::add_cell_to_line()
{
    LUnits height = 0.0f;

    Cell* pCell = get_current_cell();
    if (pCell)
    {
        LUnits width = pCell->get_width();
        height = pCell->get_height();

        m_pageCursor.x += width;
        m_availableSpace -= width;
    }

    return height;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_cell_shape(Cell* pCell, LUnits paragraphHeight)
{
    GmoObj* pGmo = pCell->create_gm_object(m_pageCursor, paragraphHeight);

    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        m_pItemMainBox->add_shape(pShape, GmoShape::k_layer_staff);
        m_pageCursor.x += pShape->get_width();
    }
    else if (pGmo->is_box())
    {
        GmoBox* pBox = dynamic_cast<GmoBox*>(pGmo);
        //UPoint pos(0.0f, 0.0f); // = m_pageCursor;
        //pos.y += pCell->shift_for_vertical_alignment(paragraphHeight);
        //USize shift(pos.x, pos.y);
        //pBox->shift_origin(shift);
        m_pItemMainBox->add_child_box(pBox);
        m_pageCursor.x += pBox->get_width();
    }
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::page_initializations(GmoBox* pMainBox)
{
    m_pItemMainBox = pMainBox;

    //no height until content added
    ImoStyle* pStyle = m_pPara->get_style();
    if (pStyle)
        m_pItemMainBox->set_height( pStyle->get_lunits_property(ImoStyle::k_margin_top) + pStyle->get_lunits_property(ImoStyle::k_border_width_top)
                                    + pStyle->get_lunits_property(ImoStyle::k_padding_top) );
    else
        m_pItemMainBox->set_height(0.0f);

    m_availableSpace = m_availableWidth;
}

//---------------------------------------------------------------------------------------
bool ParagraphLayouter::enough_space_in_box()
{
    return m_availableHeight >= m_pItemMainBox->get_height() + m_lineHeight;
}



//=======================================================================================
// CellsCreator implementation
//=======================================================================================
CellsCreator::CellsCreator(std::list<Cell*>& cells, LibraryScope& libraryScope)
    : m_cells(cells)
    , m_libraryScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
CellsCreator::~CellsCreator()
{
}

//---------------------------------------------------------------------------------------
void CellsCreator::create_cells(ImoContentObj* pImo)
{
    if (!pImo->is_box_inline())
    {
        //non-wrapping items

        if (pImo->is_textblock())
        {
            TreeNode<ImoObj>::children_iterator it;
            for (it = pImo->begin(); it != pImo->end(); ++it)
                create_cells( dynamic_cast<ImoContentObj*>( *it ) );
        }
        else if (pImo->is_text_item())
        {
            create_text_item_cells(dynamic_cast<ImoTextItem*>(pImo));
            measure_cells();
        }
        else if (pImo->is_button())
        {
            Cell* pCell = new CellButton(pImo, m_libraryScope);
            pCell->measure();
            m_cells.push_back(pCell);
        }
    }

    else
    {
        CellBox* pBox = create_wrapper_cellbox_for(pImo);
        create_and_measure_cells(pImo, pBox);
        layout_cells_in_cellbox_and_measure(pBox);
        m_cells.push_back(pBox);
    }
}

//---------------------------------------------------------------------------------------
void CellsCreator::create_and_measure_cells(ImoContentObj* pImo, CellBox* pBox)
{
    CellsCreator creator(pBox->get_cells(), m_libraryScope);

    TreeNode<ImoObj>::children_iterator it;
    for (it = pImo->begin(); it != pImo->end(); ++it)
    {
        creator.create_cells( dynamic_cast<ImoContentObj*>(*it) );
    }
}

//---------------------------------------------------------------------------------------
CellBox* CellsCreator::create_wrapper_cellbox_for(ImoContentObj* pImo)
{
    CellBox* pBox = new CellBox(pImo, m_libraryScope);
    pBox->measure();
    return pBox;
}

//---------------------------------------------------------------------------------------
void CellsCreator::layout_cells_in_cellbox_and_measure(CellBox* pBox)
{
    UPoint cursor = pBox->get_content_org();
    LUnits left = cursor.x;
    LUnits lineWidth = pBox->get_content_width();
    LUnits lineHeight = 0.0f;
    LUnits availableWidth = lineWidth;

    std::list<Cell*>& cells = pBox->get_cells();
    std::list<Cell*>::iterator it;
    for (it = cells.begin(); it != cells.end(); ++it)
    {
        Cell* pCell = *it;
        if (pCell)
        {
            LUnits width = pCell->get_width();
            if (availableWidth < width)
            {
                availableWidth = lineWidth;
                cursor.x = left;
                cursor.y += lineHeight;
                lineHeight = 0.0f;

                if (availableWidth < width)
                    break; // cell doesn't fit
            }

            pCell->set_position(cursor);
            lineHeight = max(lineHeight, pCell->get_height());

            cursor.x += width;
            availableWidth -= width;
        }
    }
    cursor.y += lineHeight;
    cursor.y += pBox->get_total_bottom_spacing();
    cursor.x += pBox->get_total_right_spacing();

    //set box size
    if (pBox->get_width() > 0.0f)
        pBox->set_height(cursor.y);
    else
        pBox->set_size(cursor.x, cursor.y);
}

//---------------------------------------------------------------------------------------
void CellsCreator::create_text_item_cells(ImoTextItem* pText)
{
    ImoStyle* pStyle = pText->get_style();
    string& text = pText->get_text();
    size_t n = text.length();
    if (n == 0)
        return;

    //initial spaces
    size_t start = 0;
    size_t length = 1;
    size_t spaces = 0;
    if (text[0] == ' ')
    {
        m_cells.push_back( new CellWord(pText, m_libraryScope, " ", pStyle));
        start = text.find_first_not_of(" ");
    }

    //words, including trailing spaces
    while (start >= 0 && start < n)
    {
        size_t stop = text.find_first_of(" ", start);
        if (stop < 0 || stop > n)
            length = n;
        else
        {
            spaces = text.find_first_not_of(" ", stop) - stop;
            if (spaces < 0)
                spaces = n;
            length = (spaces >= 1 ? stop - start + 1 : stop - start);
        }
        m_cells.push_back( new CellWord(pText, m_libraryScope,
                                        text.substr(start, length), pStyle) );
        start += length + (spaces > 1 ? spaces - 1 : 0);
    }
}

////---------------------------------------------------------------------------------------
//void CellsCreator::create_item_cell(ImoObj* pImo)
//{
//    if (pImo->is_button())
//        m_cells.push_back( new CellButton(pImo, m_libraryScope) );
//    else if (pImo->is_box_inline())
//        m_cells.push_back( new CellInlineBox(pImo, m_libraryScope) );
//}

//---------------------------------------------------------------------------------------
void CellsCreator::measure_cells()
{
    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
    {
        (*it)->measure();
    }
}


//=======================================================================================
// Cell implementation
//=======================================================================================
LUnits Cell::shift_for_vertical_alignment(LUnits paragraphHeight)
{
    if (paragraphHeight == 0.0f)
        return 0.0f;

    int valign = k_valign_middle;
    switch (valign)
    {
        case k_valign_middle:
            return (paragraphHeight - m_size.height) / 2.0f;

        case k_valign_top:
            return 0.0f;

        case k_valign_bottom:
            return (paragraphHeight - m_size.height);

        default:
            return 0.0f;
    }
}



//=======================================================================================
// CellBox implementation
//=======================================================================================
CellBox::~CellBox()
{
    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
        delete *it;
    m_cells.clear();
}

//---------------------------------------------------------------------------------------
void CellBox::measure()
{
    ImoBoxInline* pWrapper = dynamic_cast<ImoBoxInline*>(m_pCreatorImo);
    set_size( pWrapper->get_size() );
}

//---------------------------------------------------------------------------------------
GmoObj* CellBox::create_gm_object(UPoint pos, LUnits paragraphHeight)
{
    //create box
    pos.y += shift_for_vertical_alignment(paragraphHeight);
    GmoBoxInline* pBox = new GmoBoxInline(m_pCreatorImo);
    pBox->set_origin(pos);
    pBox->set_height( m_size.height );
    pBox->set_width( m_size.width );

    //create shapes
    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
    {
        GmoObj* pGmo = (*it)->create_gm_object(pos, paragraphHeight);
        add_cell_shape(pGmo, pBox);
    }

    return pBox;
}

//---------------------------------------------------------------------------------------
UPoint CellBox::get_content_org()
{
    UPoint org(0.0f, 0.0f);
    ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( m_pCreatorImo );
    ImoStyle* pStyle = pImo->get_style();
    if (pStyle)
    {
        org.x = pStyle->get_lunits_property(ImoStyle::k_margin_left);
        org.x += pStyle->get_lunits_property(ImoStyle::k_border_width_left);
        org.x += pStyle->get_lunits_property(ImoStyle::k_padding_left);

        org.y = pStyle->get_lunits_property(ImoStyle::k_margin_top);
        org.y += pStyle->get_lunits_property(ImoStyle::k_border_width_top);
        org.y += pStyle->get_lunits_property(ImoStyle::k_padding_top);
    }
    return org;
}

//---------------------------------------------------------------------------------------
LUnits CellBox::get_content_width()
{
    LUnits lineWidth = get_width() > 0.0f ? get_width() : 10000000.0f;

    ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( m_pCreatorImo );
    ImoStyle* pStyle = pImo->get_style();
    if (pStyle)
    {
        lineWidth -= pStyle->get_lunits_property(ImoStyle::k_border_width_left);
        lineWidth -= pStyle->get_lunits_property(ImoStyle::k_padding_left);
        lineWidth -= pStyle->get_lunits_property(ImoStyle::k_border_width_right);
        lineWidth -= pStyle->get_lunits_property(ImoStyle::k_padding_right);
    }
    return lineWidth;
}

//---------------------------------------------------------------------------------------
LUnits CellBox::get_total_bottom_spacing()
{
    LUnits space = 0.0f;

    ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( m_pCreatorImo );
    ImoStyle* pStyle = pImo->get_style();
    if (pStyle)
    {
        space = pStyle->get_lunits_property(ImoStyle::k_margin_bottom);
        space += pStyle->get_lunits_property(ImoStyle::k_border_width_bottom);
        space += pStyle->get_lunits_property(ImoStyle::k_padding_bottom);
    }
    return space;
}

//---------------------------------------------------------------------------------------
LUnits CellBox::get_total_right_spacing()
{
    LUnits space = 0.0f;

    ImoContentObj* pImo = dynamic_cast<ImoContentObj*>( m_pCreatorImo );
    ImoStyle* pStyle = pImo->get_style();
    if (pStyle)
    {
        space = pStyle->get_lunits_property(ImoStyle::k_margin_right);
        space += pStyle->get_lunits_property(ImoStyle::k_border_width_right);
        space += pStyle->get_lunits_property(ImoStyle::k_padding_right);
    }
    return space;
}

////---------------------------------------------------------------------------------------
//LUnits CellBox::add_cells_to_box(GmoBox* pBox)
//{
//    LUnits lineHeight = 0.0f;
//    LUnits availableWidth = m_size.width;
//    UPoint cursor;
//
//    std::list<Cell*>::iterator it;
//    for (it = m_cells.begin(); it != m_cells.end(); ++it)
//    {
//        Cell* pCell = *it;
//        if (pCell)
//        {
//            LUnits width = pCell->get_width();
//            if (availableWidth < width)
//            {
//                availableWidth = m_size.width;
//                cursor.x = 0.0f;
//                cursor.y += lineHeight;
//                lineHeight = 0.0f;
//
//                if (availableWidth < width)
//                    break; // cell doesn't fit
//            }
//
//            GmoObj* pGmo = pCell->create_gm_object(cursor, lineHeight);
//            add_cell_shape(pGmo, pBox);
//            lineHeight = max(lineHeight, pCell->get_height());
//
//            cursor.x += width;
//            availableWidth -= width;
//        }
//    }
//
//    m_size.height = cursor.y + lineHeight;
//    return m_size.height;
//}

//---------------------------------------------------------------------------------------
void CellBox::add_cell_shape(GmoObj* pGmo, GmoBox* pBox)
{
    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
    else if (pGmo->is_box())
    {
        GmoBox* pChildBox = dynamic_cast<GmoBox*>(pGmo);
        pBox->add_child_box(pChildBox);
    }
}



//=======================================================================================
// CellButton implementation
//=======================================================================================
GmoObj* CellButton::create_gm_object(UPoint pos, LUnits paragraphHeight)
{
    pos.y += shift_for_vertical_alignment(paragraphHeight);

    ////add cell origin
    //pos.x += m_org.x;
    //pos.y += m_org.y;

    return new GmoShapeButton(m_pCreatorImo, pos, m_size, m_libraryScope);
}

//---------------------------------------------------------------------------------------
void CellButton::measure()
{
    ImoButton* pButton = dynamic_cast<ImoButton*>(m_pCreatorImo);
    m_size = pButton->get_size();
}



//=======================================================================================
// CellWord implementation
//=======================================================================================
void CellWord::measure()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_pStyle->get_string_property(ImoStyle::k_font_name)
                      , m_pStyle->get_float_property(ImoStyle::k_font_size)
                      , m_pStyle->is_bold()
                      , m_pStyle->is_italic() );
    m_size.height = meter.get_font_height();
    m_size.width = meter.measure_width(m_word);
    m_descent = meter.get_descender();
    m_ascent = meter.get_ascender();
}

//---------------------------------------------------------------------------------------
GmoObj* CellWord::create_gm_object(UPoint pos, LUnits paragraphHeight)
{
    pos.y += shift_for_vertical_alignment(paragraphHeight);
    pos.y += m_size.height;     //GmoShapeWord reference point is bottom

    //add cell origin
    pos.x += m_org.x;
    pos.y += m_org.y;

    return new GmoShapeWord(m_pCreatorImo, 0, m_word, m_pStyle,
                            pos.x, pos.y, m_libraryScope);
}



//=======================================================================================
// CellInlineWrapper implementation
//=======================================================================================
CellInlineWrapper::CellInlineWrapper(ImoObj* pCreatorImo, LibraryScope& libraryScope)
    : Cell(pCreatorImo, libraryScope)
{
    m_pWrapper = dynamic_cast<ImoInlineObj*>( pCreatorImo );
}

//---------------------------------------------------------------------------------------
GmoObj* CellInlineWrapper::create_gm_object(UPoint pos, LUnits paragraphHeight)
{
    //create box
    GmoBoxInline* pBox = new GmoBoxInline(m_pCreatorImo);

    ////create shapes and add them to box
    //CellsCreator creator(m_cells, m_libraryScope);
    //std::list<ImoInlineObj*>& items = m_pWrapper->get_items();
    //std::list<ImoInlineObj*>::iterator it;
    //for (it = items.begin(); it != items.end(); ++it)
    //    creator.create_cells(*it);

    //LUnits boxHeight = add_cells_to_box(pBox);
    //pBox->set_height( boxHeight );
    //pBox->set_width( m_size.width );

    //return box
    return pBox;
}

////---------------------------------------------------------------------------------------
//void CellInlineWrapper::measure()
//{
//    //CellsCreator creator(m_cells, m_libraryScope);
//    //creator.measure_cells();
//
//    m_size = m_pWrapper->get_size();
//}

//---------------------------------------------------------------------------------------
LUnits CellInlineWrapper::add_cells_to_box(GmoBox* pBox)
{
    LUnits lineHeight = 0.0f;
    LUnits availableWidth = m_size.width;
    UPoint cursor;

    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
    {
        Cell* pCell = *it;
        if (pCell)
        {
            LUnits width = pCell->get_width();
            if (availableWidth < width)
            {
                availableWidth = m_size.width;
                cursor.x = 0.0f;
                cursor.y += lineHeight;
                lineHeight = 0.0f;

                if (availableWidth < width)
                    break; // cell doesn't fit
            }

            GmoObj* pGmo = pCell->create_gm_object(cursor, lineHeight);
            add_cell_shape(pGmo, pBox);
            lineHeight = max(lineHeight, pCell->get_height());

            cursor.x += width;
            availableWidth -= width;
        }
    }

    m_size.height = cursor.y + lineHeight;
    return m_size.height;
}

//---------------------------------------------------------------------------------------
void CellInlineWrapper::add_cell_shape(GmoObj* pGmo, GmoBox* pBox)
{
    if (pGmo->is_shape())
    {
        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
        pBox->add_shape(pShape, GmoShape::k_layer_staff);
    }
    else if (pGmo->is_box())
    {
        GmoBox* pBox = dynamic_cast<GmoBox*>(pGmo);
        pBox->add_child_box(pBox);
    }
}



////=======================================================================================
//// CellInlineBox implementation
////=======================================================================================
//CellInlineBox::CellInlineBox(ImoObj* pCreatorImo, LibraryScope& libraryScope)
//    : Cell(pCreatorImo, libraryScope)
//{
//    m_pBox = dynamic_cast<ImoInlineWrapper*>( pCreatorImo );
//}
//
////---------------------------------------------------------------------------------------
//GmoObj* CellInlineBox::create_gm_object(UPoint pos, LUnits paragraphHeight)
//{
//    //create box
//    GmoBoxInline* pBox = new GmoBoxInline();
//    pBox->set_width( m_pBox->get_width() );
//
//    //create shapes and add them to box
//    CellsCreator creator(m_cells, m_libraryScope);
//    std::list<ImoInlineObj*>& items = m_pBox->get_items();
//    std::list<ImoInlineObj*>::iterator it;
//    for (it = items.begin(); it != items.end(); ++it)
//        creator.create_cells(*it);
//
//    LUnits boxHeight = add_cells_to_box(pBox);
//    pBox->set_height( boxHeight );
//
//    //return box
//    return pBox;
//}
//
////---------------------------------------------------------------------------------------
//void CellInlineBox::measure()
//{
//    m_size = m_pBox->get_size();
//}
//
////---------------------------------------------------------------------------------------
//LUnits CellInlineBox::add_cells_to_box(GmoBox* pBox)
//{
//    LUnits lineHeight = 0.0f;
//    LUnits availableWidth = m_size.width;
//    UPoint cursor;
//
//    std::list<Cell*>::iterator it;
//    for (it = m_cells.begin(); it != m_cells.end(); ++it)
//    {
//        Cell* pCell = *it;
//        if (pCell)
//        {
//            LUnits width = pCell->get_width();
//            if (availableWidth < width)
//            {
//                availableWidth = m_size.width;
//                cursor.x = 0.0f;
//                cursor.y += lineHeight;
//                lineHeight = 0.0f;
//
//                if (availableWidth < width)
//                    break; // cell doesn't fit
//            }
//
//            GmoObj* pGmo = pCell->create_gm_object(cursor, lineHeight);
//            add_cell_shape(pGmo, pBox);
//            lineHeight = max(lineHeight, pCell->get_height());
//
//            cursor.x += width;
//            availableWidth -= width;
//        }
//    }
//
//    m_size.height = cursor.y + lineHeight;
//    return m_size.height;
//}
//
////---------------------------------------------------------------------------------------
//void CellInlineBox::add_cell_shape(GmoObj* pGmo, GmoBox* pBox)
//{
//    if (pGmo->is_shape())
//    {
//        GmoShape* pShape = dynamic_cast<GmoShape*>(pGmo);
//        pBox->add_shape(pShape, GmoShape::k_layer_staff);
//    }
//    else if (pGmo->is_box())
//    {
//        GmoBox* pBox = dynamic_cast<GmoBox*>(pGmo);
//        pBox->add_child_box(pBox);
//    }
//}
//=======================================================================================
// CellInlineBox implementation
//=======================================================================================
CellInlineBox::CellInlineBox(ImoObj* pCreatorImo, LibraryScope& libraryScope)
    : CellInlineWrapper(pCreatorImo, libraryScope)
{
    m_pBox = dynamic_cast<ImoInlineWrapper*>( pCreatorImo );
}

//---------------------------------------------------------------------------------------
GmoObj* CellInlineBox::create_gm_object(UPoint pos, LUnits paragraphHeight)
{
    //create box
    GmoBoxInline* pBox = new GmoBoxInline(m_pCreatorImo);

    //create shapes
    CellsCreator creator(m_cells, m_libraryScope);
    TreeNode<ImoObj>::children_iterator it;
    for (it = m_pBox->begin(); it != m_pBox->end(); ++it)
        creator.create_cells( dynamic_cast<ImoInlineObj*>(*it) );

    //add shapes to box
    LUnits boxHeight = add_cells_to_box(pBox);
    pBox->set_height( boxHeight );
    pBox->set_width( m_size.width );

    //return box
    return pBox;
}

//---------------------------------------------------------------------------------------
void CellInlineBox::measure()
{
    m_size = m_pBox->get_size();
}



}  //namespace lomse
