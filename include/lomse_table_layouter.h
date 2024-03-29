//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_TABLE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_TABLE_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include "lomse_layouter.h"
#include "lomse_blocks_container_layouter.h"

// other
#include <sstream>
#include <vector>

using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoTable;
class ImoTableCell;
class ImoStyles;
class ImoTableSection;
class GraphicModel;
class GmoBox;
class TableCellLayouter;
class ImoTableSection;
class TableRowLayouter;
class TableSectionLayouter;


//---------------------------------------------------------------------------------------
// TableLayouter: a layouter for tables (ImoTable objects)
class TableLayouter : public Layouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoTable* m_pTable;
    vector<LUnits> m_columnsWidth;

    //helper layouters
    TableSectionLayouter* m_headLayouter;
    TableSectionLayouter* m_bodyLayouter;

    //table size
    int m_numHeadRows;
    int m_numBodyRows;
    int m_numCols;

    //other
    LUnits m_tableWidth;

public:
    TableLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                  LibraryScope& libraryScope, ImoStyles* pStyles, bool fAddShapesToModel);
    virtual ~TableLayouter();

    //mandatory overrides
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;
    void prepare_to_start_layout() override;

protected:
    void create_sections_layouters();
    void determine_grid_size();
    void determine_num_columns();

    void determine_width_for_columns();
    void add_head();
    void add_body_row();
    bool is_body_row_ready();
    void prepare_body_row();
    bool enough_space_in_box();
};

//---------------------------------------------------------------------------------------
// TableSectionLayouter: helper layouter for a table section (head or body)
class TableSectionLayouter : public Layouter
{
protected:
    ImoTable* m_pTable;
    ImoTableSection* m_pSection;
    vector<TableCellLayouter*> m_cellLayouters;
    vector<bool> m_validCell;
    vector<LUnits>& m_columnsWidth;
    vector<int> m_rowsStart;        //data for logical rows
    int m_numSectionRows;
    int m_numTableColumns;
    LUnits m_tableWidth;
    TableRowLayouter* m_pRowLayouter;
    int m_nextLogicalRow;

public:
    TableSectionLayouter(ImoContentObj* pImo, Layouter* pParent, ImoTableSection* pSection,
                    int numRows, int numCols, LUnits tableWidth, vector<LUnits>& columnsWidth,
                    bool fAddShapesToModel);
    virtual ~TableSectionLayouter();

    //mandatory overrides
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;
    void prepare_to_start_layout() override;

    //specific
    inline void set_origin(UPoint pos) { m_pageCursor = pos; }
    LUnits add_row(GmoBox* pParentMainBox);
    bool is_row_ready();
    void prepare_row();

    //only for tests
    inline vector<TableCellLayouter*>& dbg_get_cell_layouters() { return m_cellLayouters; }
    inline vector<int>& dbg_get_row_start() { return m_rowsStart; }


protected:
    void create_cell_layouters();
};

//----------------------------------------------------------------------------------
// TableRowLayouter: layout algorithm for a logical row of cells
class TableRowLayouter : public Layouter
{
protected:
    ImoTable* m_pTable;
    vector<TableCellLayouter*>& m_cellLayouters;
    vector<LUnits>& m_columnsWidth;
    int m_iFirstRow;
    int m_numRows;
    int m_numColumns;

public:
    TableRowLayouter(ImoContentObj* pItem, Layouter* pParent,
                     vector<TableCellLayouter*>& cells,
                     vector<LUnits>& columnsWidth,
                     int iFirstRow, int numRows, int numCols,
                     bool fAddShapesToModel);
    virtual ~TableRowLayouter();

    //implementation of Layouter virtual methods
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;

protected:
    LUnits layout_cell(TableCellLayouter* pCellLayouter, GmoBox* pParentBox, UPoint pos);

};

//---------------------------------------------------------------------------------------
// TableCellLayouter: a layouter for table cells (ImoTableCell objects)
class TableCellLayouter : public BlocksContainerLayouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoTableCell* m_pCell;
    LUnits m_width;
    LUnits m_xPos;
    int m_iRow, m_iCol;

public:
    TableCellLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~TableCellLayouter() {}

    LUnits get_cell_width();
    virtual int get_rowspan();      //virtual, to override in unit tests

protected:
    friend class TableLayouter;
    friend class TableSectionLayouter;
    LUnits set_cell_width_and_position(int iRow, int iCol, LUnits xPos,
                                       vector<LUnits>& colWidths);
    void increment_colspan(int incrColSpan, int iCell, int iCol,
                           vector<LUnits>& colWidths);
};

//---------------------------------------------------------------------------------------
// TableCellSizer: algorithm to assign height to cells
class TableCellSizer
{
protected:
    vector<TableCellLayouter*>& m_cellLayouters;
    vector<LUnits>& m_heights;
    int m_iFirstRow;
    int m_numRows;
    int m_numColumns;
    vector<int> m_rowspan;

public:
    TableCellSizer(vector<TableCellLayouter*>& cells, vector<LUnits>& heights,
                   int iFirstRow, int numRows, int numCols);
    virtual ~TableCellSizer() {}

    void assign_height_and_reposition_cells();
    LUnits get_total_height();

protected:
    void create_rowspan_table();
    void compute_heights();
    void assing_height_to_cells();
    void reposition_cells();

    LUnits determine_row_height(int iRow);
    void apply_height(LUnits height, int iRow);

};


}   //namespace lomse

#endif    // __LOMSE_TABLE_LAYOUTER_H__

