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

#ifndef __LOMSE_PARAGRAPH_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_PARAGRAPH_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include "lomse_layouter.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoInlineWrapper;
class ImoInlineObj;
class ImoTextBlock;
class ImoStyles;
class ImoTextItem;
class ImoStyle;
class GraphicModel;
class GmoBox;
class Cell;
class CellWord;


//---------------------------------------------------------------------------------------
// Abstract container for an atomic item (inline object)
class Cell
{
protected:
    UPoint m_org;           //top left
    USize m_size;           //width, height
    LUnits m_baseline;      //shift from org
    ImoObj* m_pCreatorImo;
    LibraryScope& m_libraryScope;

public:
    virtual ~Cell() {}

    virtual void measure() = 0;
    virtual GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight) = 0;
    virtual LUnits shift_for_vertical_alignment(LUnits paragraphHeight);

    //size
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }
    inline void set_size(const USize& size) { m_size = size; }
    inline void set_height(LUnits height) { m_size.height = height; }
    inline void set_size(LUnits width, LUnits height) {
        m_size.width = width;
        m_size.height = height;
    }

    //position
    inline void set_position(const UPoint& pos) { m_org = pos; }
    inline UPoint& get_position() { return m_org; }

protected:
    Cell(ImoObj* pCreatorImo, LibraryScope& libraryScope)
        : m_baseline(0.0f)
        , m_pCreatorImo(pCreatorImo)
        , m_libraryScope(libraryScope)
    {
    }
};

//---------------------------------------------------------------------------------------
// Container for other Cell objects
class CellBox : public Cell
{
protected:
    std::list<Cell*> m_cells;

public:
    CellBox(ImoObj* pCreatorImo, LibraryScope& libraryScope)
        : Cell(pCreatorImo, libraryScope) {}
    virtual ~CellBox();

    void measure();
    GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);

    std::list<Cell*>& get_cells() { return m_cells; }

    UPoint get_content_org();
    LUnits get_content_width();
    LUnits get_total_bottom_spacing();
    LUnits get_total_right_spacing();


protected:
    void add_cell_shape(GmoObj* pGmo, GmoBox* pBox);

};

//---------------------------------------------------------------------------------------
// Container for a button
class CellButton : public Cell
{
public:
    CellButton(ImoObj* pCreatorImo, LibraryScope& libraryScope)
        : Cell(pCreatorImo, libraryScope) {}
    virtual ~CellButton() {}

    void measure();
    GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);
};

//---------------------------------------------------------------------------------------
//Container for a word of text
class CellWord : public Cell
{
protected:
    string m_word;
    ImoStyle* m_pStyle;
    LUnits m_descent;
    LUnits m_ascent;

public:
    CellWord(ImoObj* pCreatorImo, LibraryScope& libraryScope,
             const std::string& word, ImoStyle* pStyle)
        : Cell(pCreatorImo, libraryScope), m_word(word), m_pStyle(pStyle) {}
    virtual ~CellWord() {}

    inline const string& get_text() { return m_word; }
    inline ImoStyle* get_style() { return m_pStyle; }

    void measure();
    GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);

    //info
    inline LUnits get_descent() { return m_descent; }
    inline LUnits get_ascent() { return m_ascent; }
};


//---------------------------------------------------------------------------------------
// Wrapper for inline objs
class CellInlineWrapper : public Cell
{
protected:
    ImoInlineObj* m_pWrapper;
    std::list<Cell*> m_cells;

public:
    CellInlineWrapper(ImoObj* pCreatorImo, LibraryScope& libraryScope);
    virtual ~CellInlineWrapper() {}

    virtual void measure() = 0;
    virtual GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);

protected:
    LUnits add_cells_to_box(GmoBox* pBox);
    void add_cell_shape(GmoObj* pGmo, GmoBox* pBox);
};

//---------------------------------------------------------------------------------------
// Container for an InlineBox
class CellInlineBox : public CellInlineWrapper
{
protected:
    ImoInlineWrapper* m_pBox;

public:
    CellInlineBox(ImoObj* pCreatorImo, LibraryScope& libraryScope);
    virtual ~CellInlineBox() {}

    void measure();
    GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);
};

//class CellInlineBox : public Cell
//{
//protected:
//    ImoInlineWrapper* m_pBox;
//    std::list<Cell*> m_cells;
//
//public:
//    CellInlineBox(ImoObj* pCreatorImo, LibraryScope& libraryScope);
//    virtual ~CellInlineBox() {}
//
//    void measure();
//    GmoObj* create_gm_object(UPoint pos, LUnits paragraphHeight);
//
//protected:
//    LUnits add_cells_to_box(GmoBox* pBox);
//    void add_cell_shape(GmoObj* pGmo, GmoBox* pBox);
//};


//---------------------------------------------------------------------------------------
// CellsCreator: splits paragraph content, creating cells for each atomic content
class CellsCreator
{
protected:
    std::list<Cell*>& m_cells;
    LibraryScope& m_libraryScope;
    UPoint m_cursor;                //current position. Relative to BoxDocPage

public:
    CellsCreator(std::list<Cell*>& cells, LibraryScope& libraryScope);
    virtual ~CellsCreator();

    void create_cells(ImoContentObj* pImo);

protected:
    CellBox* create_wrapper_cellbox_for(ImoContentObj* pImo);
    void create_and_measure_cells(ImoContentObj* pImo, CellBox* pBox);
    void layout_cells_in_cellbox_and_measure(CellBox* pBox);

    void create_text_item_cells(ImoTextItem* pText);
    //void create_item_cell(ImoObj* pImo);
    void measure_cells();

};



//---------------------------------------------------------------------------------------
// ParagraphLayouter: layouts a paragraph
class ParagraphLayouter : public Layouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoTextBlock* m_pPara;
    std::list<Cell*> m_cells;

public:
    ParagraphLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                      LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~ParagraphLayouter();

    //virtual methods in base class
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);
    void prepare_to_start_layout();

protected:
    void page_initializations(GmoBox* pMainBox);
    void create_cells();

    bool enough_space_in_box();
    LUnits add_cell_to_line();
    void add_cell_shape(Cell* pCell, LUnits height);

    //helper: cells traversing
    std::list<Cell*>::iterator m_itCells;
    inline void point_to_first_cell() { m_itCells = m_cells.begin(); }
    inline bool more_cells() { return m_itCells != m_cells.end(); }
    inline void next_cell() { ++m_itCells; }
    inline Cell* get_current_cell() { return *m_itCells; };

    //helper: space in current line
    LUnits m_availableSpace;
    inline bool space_in_line() { return m_availableSpace > (*m_itCells)->get_width(); }

    //helper: info about next line to add to paragraph
    std::list<Cell*>::iterator m_itStart;       //first cell for this line
    std::list<Cell*>::iterator m_itEnd;         //first cell for next line
    LUnits m_lineHeight;                        //line height
    inline void initialize_lines() { m_itStart = m_cells.end(); }
    inline bool is_line_ready() { return m_itStart != m_cells.end(); }
    void prepare_line();
    void add_line();

};


}   //namespace lomse

#endif    // __LOMSE_PARAGRAPH_LAYOUTER_H__

