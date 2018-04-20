//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_table_layouter.h"
#include "lomse_build_options.h"

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_logger.h"

namespace lomse
{

//=======================================================================================
// TableLayouter implementation
//=======================================================================================
TableLayouter::TableLayouter(ImoContentObj* pItem, Layouter* pParent,
                             GraphicModel* pGModel, LibraryScope& libraryScope,
                             ImoStyles* pStyles, bool fAddShapesToModel)
    : Layouter(pItem, pParent, pGModel, libraryScope, pStyles, fAddShapesToModel)
    , m_libraryScope(libraryScope)
    , m_pTable( static_cast<ImoTable*>(pItem) )
    , m_headLayouter(nullptr)
    , m_bodyLayouter(nullptr)
{
}

//---------------------------------------------------------------------------------------
TableLayouter::~TableLayouter()
{
    delete m_headLayouter;
    delete m_bodyLayouter;
}

//---------------------------------------------------------------------------------------
void TableLayouter::prepare_to_start_layout()
{
    Layouter::prepare_to_start_layout();

    determine_grid_size();
    determine_width_for_columns();
    create_sections_layouters();
}

//---------------------------------------------------------------------------------------
void TableLayouter::create_sections_layouters()
{
    if (m_numHeadRows > 0)
    {
        ImoTableHead* pHead = m_pTable->get_head();
        m_headLayouter = LOMSE_NEW TableSectionLayouter(m_pTable, this, pHead, m_numHeadRows,
                                                   m_numCols, m_tableWidth, m_columnsWidth,
                                                   m_fAddShapesToModel);
        m_headLayouter->prepare_to_start_layout();
    }

    if (m_numBodyRows > 0)
    {
        ImoTableBody* pBody = m_pTable->get_body();
        m_bodyLayouter = LOMSE_NEW TableSectionLayouter(m_pTable, this, pBody, m_numBodyRows,
                                                   m_numCols, m_tableWidth, m_columnsWidth,
                                                   m_fAddShapesToModel);
        m_bodyLayouter->prepare_to_start_layout();
    }
}

//---------------------------------------------------------------------------------------
void TableLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, "");

    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_box() method must always continue laying out from current state.

    set_cursor_and_available_space();

    add_head();       //TO_FIX: It is assumed that head always fit in current page

    if (m_bodyLayouter)
    {
        m_bodyLayouter->set_origin(m_pageCursor);

        if (!is_body_row_ready())
            prepare_body_row();

        if (!is_body_row_ready())
        {
            //problem or empty body. Terminate table laying out
            set_layout_is_finished(true);
            return;
        }

        //loop to add rows
        while(is_body_row_ready() && enough_space_in_box())
        {
            add_body_row();
            prepare_body_row();
        }
    }

    bool fMoreRows = (m_bodyLayouter && m_bodyLayouter->is_row_ready());
    set_layout_is_finished( !fMoreRows );
}

//---------------------------------------------------------------------------------------
void TableLayouter::create_main_box(GmoBox* pParentBox, UPoint pos,
                                    LUnits UNUSED(width), LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxTable(m_pTable);
    pParentBox->add_child_box(m_pItemMainBox);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(m_tableWidth);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
void TableLayouter::determine_grid_size()
{
    ImoTableHead* pHead = m_pTable->get_head();
    m_numHeadRows = (pHead ? pHead->get_num_children() : 0);

    ImoTableBody* pBody = m_pTable->get_body();
    m_numBodyRows = (pBody ? pBody->get_num_children() : 0);

    m_numCols = m_pTable->get_num_columns();
    if (m_numCols == 0)
        determine_num_columns();
}

//---------------------------------------------------------------------------------------
void TableLayouter::determine_num_columns()
{
    //TODO: numColumns should be a datum provided by ImoTable and, for it, this method
    // should be moved there. But this would require a table structurizer object and to
    // follow a table open/close protocol for table modifications.

    TreeNode<ImoObj>::children_iterator itRow;

    ImoTableHead* pHead = m_pTable->get_head();
    if (pHead)
    {
        for (itRow = pHead->begin(); itRow != pHead->end(); ++itRow)
        {
            ImoTableRow* pRow = static_cast<ImoTableRow*>( *itRow );
            ImoContent* pWrapper = pRow->get_content();
            int numCells = 0;
            TreeNode<ImoObj>::children_iterator itCell;
            for (itCell = pWrapper->begin(); itCell != pWrapper->end(); ++itCell)
            {
                ImoTableCell* pCell = static_cast<ImoTableCell*>( *itCell );
                numCells += pCell->get_colspan();
            }
            m_numCols = max(m_numCols, numCells);
        }
    }

    ImoTableBody* pBody = m_pTable->get_body();
    for (itRow = pBody->begin(); itRow != pBody->end(); ++itRow)
    {
        ImoTableRow* pRow = static_cast<ImoTableRow*>( *itRow );
        ImoContent* pWrapper = pRow->get_content();
        int numCells = 0;
        TreeNode<ImoObj>::children_iterator itCell;
        for (itCell = pWrapper->begin(); itCell != pWrapper->end(); ++itCell)
        {
            ImoTableCell* pCell = static_cast<ImoTableCell*>( *itCell );
            numCells += pCell->get_colspan();
        }
        m_numCols = max(m_numCols, numCells);
    }
}

//---------------------------------------------------------------------------------------
void TableLayouter::determine_width_for_columns()
{
    //TODO: For now width will be assigned based on column style.

    m_columnsWidth.resize(m_numCols);
    m_tableWidth = 0.0f;
    std::list<ImoStyle*>& cols = m_pTable->get_column_styles();
    std::list<ImoStyle*>::iterator it;
    int iCol = 0;
    for (it = cols.begin(); it != cols.end(); ++it, ++iCol)
    {
        m_columnsWidth[iCol] = (*it)->table_col_width();
        m_tableWidth += m_columnsWidth[iCol];
    }
}

//---------------------------------------------------------------------------------------
void TableLayouter::add_head()
{
    if (m_numHeadRows > 0)
    {
        m_headLayouter->set_origin(m_pageCursor);

        if (!m_headLayouter->is_row_ready())
            m_headLayouter->prepare_row();

        //loop to add rows
        while(m_headLayouter->is_row_ready())
        {
            LUnits height = m_headLayouter->add_row(m_pItemMainBox);
            m_pageCursor.y += height;
            m_availableHeight -= height;

            m_headLayouter->prepare_row();
        }
    }
}

//---------------------------------------------------------------------------------------
void TableLayouter::add_body_row()
{
    if (m_bodyLayouter)
    {
        LUnits height = m_bodyLayouter->add_row(m_pItemMainBox);
        m_pageCursor.y += height;
        m_availableHeight -= height;
    }
}

//---------------------------------------------------------------------------------------
bool TableLayouter::is_body_row_ready()
{
    return m_bodyLayouter->is_row_ready();
}

//---------------------------------------------------------------------------------------
void TableLayouter::prepare_body_row()
{
    m_bodyLayouter->prepare_row();
}

//---------------------------------------------------------------------------------------
bool TableLayouter::enough_space_in_box()
{
    return true;        //TODO: TableLayouter::enough_space_in_box
}



//=======================================================================================
// TableSectionLayouter implementation
//=======================================================================================
TableSectionLayouter::TableSectionLayouter(ImoContentObj* pItem, Layouter* pParent,
                                 ImoTableSection* pSection, int numRows,
                                 int numCols, LUnits tableWidth,
                                 vector<LUnits>& columnsWidth,
                                 bool fAddShapesToModel)
    : Layouter(pItem, pParent, pParent->get_graphic_model(),
               pParent->get_library_scope(), pParent->get_styles(), fAddShapesToModel )
    , m_pTable( static_cast<ImoTable*>(pItem) )
    , m_pSection(pSection)
    , m_columnsWidth(columnsWidth)
    , m_numSectionRows(numRows)
    , m_numTableColumns(numCols)
    , m_tableWidth(tableWidth)
    , m_pRowLayouter(nullptr)
{
}

//---------------------------------------------------------------------------------------
TableSectionLayouter::~TableSectionLayouter()
{
    delete m_pRowLayouter;

    //delete cell layouters
    vector<TableCellLayouter*>::iterator it;
    for (it = m_cellLayouters.begin(); it != m_cellLayouters.end(); ++it)
        delete *it;
}

//---------------------------------------------------------------------------------------
void TableSectionLayouter::prepare_to_start_layout()
{
    Layouter::prepare_to_start_layout();
    create_cell_layouters();
    m_nextLogicalRow = 0;
}

//---------------------------------------------------------------------------------------
void TableSectionLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, "");
}

//---------------------------------------------------------------------------------------
void TableSectionLayouter::create_main_box(GmoBox* UNUSED(pParentBox),
                                           UPoint UNUSED(pos), LUnits UNUSED(width),
                                           LUnits UNUSED(height))
{
    LOMSE_LOG_DEBUG(Logger::k_layout, "");
}

//---------------------------------------------------------------------------------------
void TableSectionLayouter::create_cell_layouters()
{
    //AWARE: This method is a lier. It does many things:
    // - creates the m_rowsStart table, that splits the section into logical rows
    // - creates the grid of cell layouters (m_cellLayouters)
    // - assigns x position to the cells

    int numCells = m_numSectionRows * m_numTableColumns;
    m_cellLayouters.assign(numCells, (TableCellLayouter*)nullptr);

    vector<bool> freeCell;
    freeCell.assign(numCells, true);

    TreeNode<ImoObj>::children_iterator itRow;
    int iLastCell;
    int iRow = 0;
    int iNextLogicalRow = 0;
    LUnits xPos = 0.0f;     //position relative to section top-left corner
    for (itRow = m_pSection->begin(); itRow != m_pSection->end(); ++itRow)
    {
        if (iNextLogicalRow == iRow)
            m_rowsStart.push_back(iRow);

        ImoTableRow* pRow = static_cast<ImoTableRow*>( *itRow );
        int iCell = iRow*m_numTableColumns;
        iLastCell = iCell;
        ImoContent* pWrapper = pRow->get_content();
        TreeNode<ImoObj>::children_iterator itCell;
        int iCol = 0;
        for (itCell = pWrapper->begin(); itCell != pWrapper->end(); ++itCell, ++iCell, ++iCol)
        {
            while( iCell < numCells && !freeCell[iCell])
                ++iCell, ++iCol;

            ImoTableCell* pCell = static_cast<ImoTableCell*>( *itCell );
            iLastCell = iCell;
            m_cellLayouters[iCell] = static_cast<TableCellLayouter*>(
                                        LayouterFactory::create_layouter(pCell, this) );
            xPos += m_cellLayouters[iCell]->set_cell_width_and_position(iRow, iCol, xPos,
                                                                        m_columnsWidth);

            //mark used cells
            for (int i = pCell->get_rowspan(); i > 0; --i)
            {
                int k = iCell + m_numTableColumns * (i - 1);
                for (int j = pCell->get_colspan(); j > 0; --j)
                    freeCell[k + j -1] = false;
            }

            iNextLogicalRow = max(iNextLogicalRow, iRow + pCell->get_rowspan());
        }

        //if last processed cell doesn't fill row and next cell is not used, assume last
        //processed cell has implicit colspan
        iRow++;
        if (iCell < numCells && freeCell[iCell])
        {
            int colSpan = 0;
            int iStart = iCell;
            int iStartNextCol = iRow * m_numTableColumns;
            while( iStart < numCells && iStart < iStartNextCol)
            {
                freeCell[iStart++] = false;
                colSpan++;
            }
            if (colSpan > 0)
            {
                int iCol = iLastCell % m_numTableColumns;
                m_cellLayouters[iLastCell]->increment_colspan(colSpan, iCell, iCol,
                                                              m_columnsWidth);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits TableSectionLayouter::add_row(GmoBox* pParentMainBox)
{
    GmoBox* pRowBox = m_pRowLayouter->get_item_main_box();
    pParentMainBox->add_child_box(pRowBox);

    LUnits height = pRowBox->get_height();
    m_pageCursor.y += height;
    m_availableHeight -= height;

    delete m_pRowLayouter;
    m_pRowLayouter = nullptr;

    return height;
}

//---------------------------------------------------------------------------------------
bool TableSectionLayouter::is_row_ready()
{
    return (m_pRowLayouter != nullptr);
}

//---------------------------------------------------------------------------------------
void TableSectionLayouter::prepare_row()
{
    //layouts a logical row. Result is not added to graphical model.

    if (m_nextLogicalRow >= int(m_rowsStart.size()) )
        return;     //no more rows

    int iFirstRow = m_rowsStart[m_nextLogicalRow++];
    int numRows = (m_nextLogicalRow < int(m_rowsStart.size())
                        ? m_rowsStart[m_nextLogicalRow] - iFirstRow
                        : m_numSectionRows - iFirstRow );

    m_pRowLayouter = LOMSE_NEW TableRowLayouter(m_pTable, this, m_cellLayouters,
                                                m_columnsWidth, iFirstRow, numRows,
                                                m_numTableColumns, m_fAddShapesToModel);

    GmoBox* pParentMainBox = m_pParentLayouter->get_item_main_box();
    m_pRowLayouter->create_main_box(pParentMainBox, m_pageCursor, m_tableWidth,
                                    100000000.0f /*no height limit*/);
    m_pRowLayouter->layout_in_box();
    m_pRowLayouter->set_box_height();
}


//=======================================================================================
// TableCellLayouter implementation
//=======================================================================================
TableCellLayouter::TableCellLayouter(ImoContentObj* pItem, Layouter* pParent,
                                     GraphicModel* pGModel, LibraryScope& libraryScope,
                                     ImoStyles* pStyles)
    : BlocksContainerLayouter(pItem, pParent, pGModel, libraryScope, pStyles,
                              false /*do not add shapes to model*/)
    , m_libraryScope(libraryScope)
    , m_pCell( static_cast<ImoTableCell*>(pItem) )
    , m_width(0.0f)
    , m_xPos(0.0f)
    , m_iRow(0)
    , m_iCol(0)
{
}

//---------------------------------------------------------------------------------------
LUnits TableCellLayouter::get_cell_width()
{
    return m_width;
}

//---------------------------------------------------------------------------------------
LUnits TableCellLayouter::set_cell_width_and_position(int iRow, int iCol, LUnits xPos,
                                                      vector<LUnits>& colWidths)
{
    m_xPos = xPos;
    m_iRow = iRow;
    m_iCol = iCol;

    m_width = 0.0f;
    int colspan = m_pCell->get_colspan();
    for (int i=0; i < colspan; ++i, ++iCol)
        m_width += colWidths[iCol];

    return m_width;
}

//---------------------------------------------------------------------------------------
int TableCellLayouter::get_rowspan()
{
    return m_pCell->get_rowspan();
}

//---------------------------------------------------------------------------------------
void TableCellLayouter::increment_colspan(int incrColSpan, int UNUSED(iCell), int iCol,
                                          vector<LUnits>& colWidths)
{
    m_width = 0.0f;
    int colspan = m_pCell->get_colspan() + incrColSpan;
    m_pCell->set_colspan(colspan);
    for (int i=0; i < colspan; ++i, ++iCol)
        m_width += colWidths[iCol];
}



//=======================================================================================
// TableRowLayouter implementation
//=======================================================================================
TableRowLayouter::TableRowLayouter(ImoContentObj* pItem, Layouter* pParent,
                                   vector<TableCellLayouter*>& cells,
                                   vector<LUnits>& columnsWidth,
                                   int iFirstRow, int numRows, int numColumns,
                                   bool fAddShapesToModel)
    : Layouter(pItem, pParent, pParent->get_graphic_model(),
               pParent->get_library_scope(), pParent->get_styles(), fAddShapesToModel)
    , m_pTable( static_cast<ImoTable*>(pItem) )
    , m_cellLayouters(cells)
    , m_columnsWidth(columnsWidth)
    , m_iFirstRow(iFirstRow)
    , m_numRows(numRows)
    , m_numColumns(numColumns)
{
}

//---------------------------------------------------------------------------------------
TableRowLayouter::~TableRowLayouter()
{
}

//---------------------------------------------------------------------------------------
void TableRowLayouter::layout_in_box()
{
    LOMSE_LOG_DEBUG(Logger::k_layout, "");

    set_cursor_and_available_space();
    LUnits yPos = m_pageCursor.y;

    //loop to layout the cells in this rows group range
    vector<LUnits> cellHeights;
    cellHeights.assign(m_cellLayouters.size(), 0.0f);

    int iRow = m_iFirstRow;
    for (int i=0; i < m_numRows; ++i, ++iRow)
    {
        LUnits xPos = m_pItemMainBox->get_content_left();
        for (int iCol=0; iCol < m_numColumns; ++iCol)
        {
            int iCell = iRow * m_numColumns + iCol;
            if (m_cellLayouters[iCell] != nullptr)
            {
                LUnits height = layout_cell(m_cellLayouters[iCell], m_pItemMainBox,
                                            UPoint(xPos, m_pageCursor.y));
                cellHeights[iCell] = height;
            }
            xPos += m_columnsWidth[iCol];
        }
    }

    //set height and final position of cells
    TableCellSizer sizer(m_cellLayouters, cellHeights, m_iFirstRow,
                         m_numRows, m_numColumns);
    sizer.assign_height_and_reposition_cells();
    yPos += sizer.get_total_height();

    //update cursor and available space
    LUnits height = yPos - m_pageCursor.y;
    m_pageCursor.y = yPos;
    m_availableHeight -= height;

    set_layout_is_finished(true);
}

//---------------------------------------------------------------------------------------
void TableRowLayouter::create_main_box(GmoBox* UNUSED(pParentBox), UPoint pos,
                                       LUnits width, LUnits height)
{
    m_pItemMainBox = LOMSE_NEW GmoBoxTableRows(m_pTable);

    m_pItemMainBox->set_origin(pos);
    m_pItemMainBox->set_width(width);
    m_pItemMainBox->set_height(height);
}

//---------------------------------------------------------------------------------------
LUnits TableRowLayouter::layout_cell(TableCellLayouter* pCellLayouter,
                                     GmoBox* pParentBox, UPoint pos)
{
    pCellLayouter->prepare_to_start_layout();
    LUnits width = pCellLayouter->get_cell_width();
    pCellLayouter->create_main_box(pParentBox, pos, width, m_availableHeight);
    pCellLayouter->layout_in_box();
    return pCellLayouter->set_box_height();
}


//=======================================================================================
// TableCellSizer implementation
//=======================================================================================
TableCellSizer::TableCellSizer(vector<TableCellLayouter*>& cells,
                               vector<LUnits>& heights, int iFirstRow,
                               int numRows, int numCols)
    : m_cellLayouters(cells)
    , m_heights(heights)
    , m_iFirstRow(iFirstRow)
    , m_numRows(numRows)
    , m_numColumns(numCols)
{
}

//---------------------------------------------------------------------------------------
void TableCellSizer::assign_height_and_reposition_cells()
{
    create_rowspan_table();
    compute_heights();
    assing_height_to_cells();
    reposition_cells();
}

//---------------------------------------------------------------------------------------
LUnits TableCellSizer::get_total_height()
{
    LUnits height = 0.0f;
    int iCell = m_iFirstRow * m_numColumns;
    for (int iRow=0; iRow < m_numRows; ++iRow)
    {
        height += m_heights[iCell];
        iCell += m_numColumns;
    }
    return height;
}

//---------------------------------------------------------------------------------------
void TableCellSizer::create_rowspan_table()
{
    m_rowspan.assign(m_numRows * m_numColumns, 0);
    for (int iRow=0; iRow < m_numRows; ++iRow)
    {
        int iCell = (m_iFirstRow + iRow) * m_numColumns;
        int iTable = iRow * m_numColumns;
        for (int iCol=0; iCol < m_numColumns; ++iCol, ++iCell, ++iTable)
        {
            if (m_cellLayouters[iCell] != nullptr)
            {
                int r = m_cellLayouters[iCell]->get_rowspan();
                int iT = iTable;
                for (int i=r; i > 0; --i)
                {
                    m_rowspan[iT] = i-1;
                    iT += m_numColumns;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void TableCellSizer::compute_heights()
{
    for (int iRow=0; iRow < m_numRows; ++iRow)
    {
        LUnits height = determine_row_height(iRow);
        apply_height(height, iRow);
    }
}

//---------------------------------------------------------------------------------------
LUnits TableCellSizer::determine_row_height(int iRow)
{
    LUnits maxHeight = 0.0f;
    int iCell = (m_iFirstRow + iRow) * m_numColumns;
    int iTable = iRow * m_numColumns;
    for (int iCol=0; iCol < m_numColumns; ++iCol, ++iCell, ++iTable)
    {
        if (m_rowspan[iTable] == 0)
            maxHeight = max(maxHeight, m_heights[iCell]);
    }
    return maxHeight;
}

//---------------------------------------------------------------------------------------
void TableCellSizer::apply_height(LUnits height, int iRow)
{
    int iCell = (m_iFirstRow + iRow) * m_numColumns;
    int iTable = iRow * m_numColumns;
    for (int iCol=0; iCol < m_numColumns; ++iCol, ++iCell, ++iTable)
    {
        LUnits diff = m_heights[iCell] - height;
        m_heights[iCell] = height;
        if (m_rowspan[iTable] > 0)
            m_heights[iCell + m_numColumns] = diff;
    }
}

//---------------------------------------------------------------------------------------
void TableCellSizer::assing_height_to_cells()
{
    for (int iRow=0; iRow < m_numRows; ++iRow)
    {
        int iCell = (m_iFirstRow + iRow) * m_numColumns;
        for (int iCol=0; iCol < m_numColumns; ++iCol, ++iCell)
        {
            if (m_cellLayouters[iCell] != nullptr)
            {
                int iH = iCell;
                LUnits height = m_heights[iH];
                int rowspan = m_cellLayouters[iCell]->get_rowspan();
                while (rowspan > 1)
                {
                    iH += m_numColumns;
                    height += m_heights[iH];
                    --rowspan;
                }
                GmoBox* pCellBox = m_cellLayouters[iCell]->get_item_main_box();
                pCellBox->set_height(height);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
void TableCellSizer::reposition_cells()
{
    //AWARE: row 0 is correctly positioned, so we start moving from row 1.
    //In horizontal cells are correctly positioned, so shift is only in vertical.

    int iH = m_iFirstRow * m_numColumns;
    USize shift(0.0f, 0.0f);
    for (int iRow=1; iRow < m_numRows; ++iRow)
    {
        shift.height += m_heights[iH];
        int iCell = (m_iFirstRow + iRow) * m_numColumns;
        for (int iCol=0; iCol < m_numColumns; ++iCol, ++iCell)
        {
            if (m_cellLayouters[iCell] != nullptr)
            {
                GmoBox* pCellBox = m_cellLayouters[iCell]->get_item_main_box();
                pCellBox->shift_origin(shift);
            }
        }
        iH += m_numColumns;
    }
}



}  //namespace lomse
