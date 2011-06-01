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
#include "lomse_content_layouter.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoTextBlock;
class ImoStyles;
class ImoTextItem;
class ImoTextStyleInfo;
class TextMeter;
class GraphicModel;
class GmoBox;
class Cell;
class CellWord;


//---------------------------------------------------------------------------------------
//Container for a paragraph atomic item (word, image, line, etc.). Abstract
class Cell
{
protected:
    std::list<GmoShape*> m_shapes;
    UPoint m_org;           //top left
    USize m_size;           //width, height
    LUnits m_baseline;      //shift from org

public:
    virtual ~Cell() {}

    virtual void measure(TextMeter& meter) = 0;

    //info
    inline LUnits get_width() { return m_size.width; }
    inline LUnits get_height() { return m_size.height; }

protected:
    Cell() : m_baseline(0.0f) {}

};

//---------------------------------------------------------------------------------------
//Container for a word of text
class CellWord : public Cell
{
protected:
    string m_word;
    ImoTextStyleInfo* m_pStyle;
    LUnits m_descent;
    LUnits m_ascent;

public:
    CellWord(const std::string& word, ImoTextStyleInfo* pStyle)
        : Cell(), m_word(word), m_pStyle(pStyle) {}
    virtual ~CellWord() {}

    inline const string& get_text() { return m_word; }
    inline ImoTextStyleInfo* get_style() { return m_pStyle; }

    void measure(TextMeter& meter);

    //info
    inline LUnits get_descent() { return m_descent; }
    inline LUnits get_ascent() { return m_ascent; }
};


//---------------------------------------------------------------------------------------
// CellsCreator: splits paragraph content, creating cells for each atomic content
class CellsCreator
{
protected:
    ImoTextBlock* m_pPara;
    std::list<Cell*>& m_cells;
    LibraryScope& m_libraryScope;
    UPoint m_cursor;                //current position. Relative to BoxDocPage

public:
    CellsCreator(ImoTextBlock* pPara, std::list<Cell*>& cells,
                 LibraryScope& libraryScope);
    virtual ~CellsCreator();

    void create_cells();

protected:
    void create_text_item_cells(ImoTextItem* pText);
    void measure_cells();
    void align_cells();

};



//---------------------------------------------------------------------------------------
// ParagraphLayouter: layouts a paragraph
class ParagraphLayouter : public ContentLayouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoTextBlock* m_pPara;
    ImoStyles* m_pStyles;
    GmoBox* m_pMainBox;
    std::list<Cell*> m_cells;

public:
    ParagraphLayouter(ImoContentObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope,
                      ImoStyles* pStyles);
    virtual ~ParagraphLayouter();

    //virtual methods in base class
    void layout_in_page(GmoBox* pContainerBox);
    GmoBox* create_main_box();
    void prepare_to_start_layout();

protected:
    void page_initializations(GmoBox* pMainBox);
    void create_cells();
    void add_line();

    bool enough_space_in_box();
    LUnits add_cell_to_line();
    void add_cell_shape(Cell* pCell, LUnits height);

    //helper: cells traversing
    std::list<Cell*>::iterator m_itCells;
    inline void point_to_first_cell() { m_itCells = m_cells.begin(); }
    inline bool more_cells() { return m_itCells != m_cells.end(); }
    inline void next_cell() { ++m_itCells; }
    inline Cell* get_current_cell() { return *m_itCells; };

    //helper: space and position in current line
    UPoint m_cursor;                //current position. Relative to BoxDocPage
    LUnits m_availableSpace;
    inline bool space_in_line() { return m_availableSpace > (*m_itCells)->get_width(); }

};


}   //namespace lomse

#endif    // __LOMSE_PARAGRAPH_LAYOUTER_H__

