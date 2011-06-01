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

#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_text_engraver.h"
#include "lomse_text_engraver.h"
#include "lomse_shape_text.h"
#include "lomse_calligrapher.h"


namespace lomse
{

//=======================================================================================
// ParagraphLayouter implementation
//=======================================================================================
ParagraphLayouter::ParagraphLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                                     LibraryScope& libraryScope, ImoStyles* pStyles)
    : ContentLayouter(pImo, pGModel)
    , m_libraryScope(libraryScope)
    , m_pPara( dynamic_cast<ImoTextBlock*>(pImo) )
    , m_pStyles(pStyles)
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
    ContentLayouter::prepare_to_start_layout();
    create_cells();
    point_to_first_cell();
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::layout_in_page(GmoBox* pMainBox)
{
    //AWARE: This method is invoked to layout a page. If there are more pages to
    //layout, it will be invoked more times. Therefore, this method must not initialize
    //anything. All initializations must be done in 'prepare_to_start_layout()'.
    //layout_in_page() method must always continue layouting from current state.

    page_initializations(pMainBox);

    while(enough_space_in_box() && more_cells())
    {
        add_line();
    }

    bool fMoreText = more_cells();
    if (!fMoreText)
    {
        //copy shapes in BoxDocPage
        pMainBox->store_shapes_in_doc_page();
    }
    set_layout_is_finished( !fMoreText );
}

//---------------------------------------------------------------------------------------
GmoBox* ParagraphLayouter::create_main_box()
{
    m_pMainBox = new GmoBoxParagraph();
    return m_pMainBox;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::create_cells()
{
    CellsCreator creator(m_pPara, m_cells, m_libraryScope);
    creator.create_cells();
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_line()
{
    LUnits height = 0.0f;

    std::list<Cell*>::iterator itStart = m_cells.end();
    std::list<Cell*>::iterator itEnd = m_cells.end();

    if (more_cells())
        itStart = m_itCells;

    UPoint pos = m_cursor;
    while(more_cells() && space_in_line())
    {
        LUnits cellHeight = add_cell_to_line();
        height = max(height, cellHeight);
        next_cell();
    }
    itEnd = m_itCells;

    m_cursor = pos;
    std::list<Cell*>::iterator it;
    for(it = itStart; it != itEnd; ++it)
    {
        add_cell_shape(*it, height);
    }

    height *= 1.5f;
    m_pMainBox->set_height( m_pMainBox->get_height() + height );

    //prepare for next line
    m_cursor.x = m_pMainBox->get_left();
    m_cursor.y += height;
    m_availableSpace = m_pMainBox->get_width();
}

//---------------------------------------------------------------------------------------
LUnits ParagraphLayouter::add_cell_to_line()
{
    LUnits height = 0.0f;

    Cell* pCell = get_current_cell();
    CellWord* pWord = dynamic_cast<CellWord*>(pCell);
    if (pWord)
    {
        LUnits width = pWord->get_width();
        height = pWord->get_height();

        m_cursor.x += width;
        m_availableSpace -= width;
    }

    return height;
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::add_cell_shape(Cell* pCell, LUnits height)
{
    CellWord* pWord = dynamic_cast<CellWord*>(pCell);
    if (pWord)
    {
        LUnits width = pWord->get_width();

        ImoTextStyleInfo* pStyle = pWord->get_style();
        LUnits xLeft = m_cursor.x;
        LUnits yTop = m_cursor.y + height;
        GmoShape* pShape = new GmoShapeWord(m_pPara, 0, pWord->get_text(), pStyle,
                                            xLeft, yTop, m_libraryScope);
        m_pMainBox->add_shape(pShape, GmoShape::k_layer_staff);

        m_cursor.x += width;
    }
}

//---------------------------------------------------------------------------------------
void ParagraphLayouter::page_initializations(GmoBox* pMainBox)
{
    m_pMainBox = pMainBox;
    m_pMainBox->set_height(0.0f);        //no height until content added

    m_cursor.x = pMainBox->get_left();
    m_cursor.y = pMainBox->get_top();
    m_availableSpace = pMainBox->get_width();
    //is_first_system_in_page(true);
}

//---------------------------------------------------------------------------------------
bool ParagraphLayouter::enough_space_in_box()
{
    //TODO
    return true;
}



//=======================================================================================
// CellsCreator implementation
//=======================================================================================
CellsCreator::CellsCreator(ImoTextBlock* pPara, std::list<Cell*>& cells,
                           LibraryScope& libraryScope)
    : m_pPara(pPara)
    , m_cells(cells)
    , m_libraryScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
CellsCreator::~CellsCreator()
{
}

//---------------------------------------------------------------------------------------
void CellsCreator::create_cells()
{
    std::list<ImoContentObj*>& items = m_pPara->get_items();
    std::list<ImoContentObj*>::iterator it;
    for (it = items.begin(); it != items.end(); ++it)
    {
        if ((*it)->is_text_item())
            create_text_item_cells(dynamic_cast<ImoTextItem*>(*it));
    }

    measure_cells();
    align_cells();
}

//---------------------------------------------------------------------------------------
void CellsCreator::create_text_item_cells(ImoTextItem* pText)
{
    ImoTextStyleInfo* pStyle = pText->get_style();
    string& text = pText->get_text();
    int n = text.length();
    if (n == 0)
        return;

    //initial spaces
    int start = 0;
    int length = 1;
    int spaces = 0;
    if (text[0] == ' ')
    {
        m_cells.push_back( new CellWord(" ", pStyle));
        start = text.find_first_not_of(" ");
    }

    //words, including trailing spaces
    while (start >= 0 && start < n)
    {
        int stop = text.find_first_of(" ", start);
        if (stop < 0 || stop > n)
            length = n;
        else
        {
            spaces = text.find_first_not_of(" ", stop) - stop;
            if (spaces < 0)
                spaces = n;
            length = (spaces >= 1 ? stop - start + 1 : stop - start);
        }
        m_cells.push_back( new CellWord(text.substr(start, length), pStyle) );
        start += length + (spaces > 1 ? spaces - 1 : 0);
    }
}

//---------------------------------------------------------------------------------------
void CellsCreator::measure_cells()
{
    TextMeter meter(m_libraryScope);

    std::list<Cell*>::iterator it;
    for (it = m_cells.begin(); it != m_cells.end(); ++it)
    {
        (*it)->measure(meter);
    }
}

//---------------------------------------------------------------------------------------
void CellsCreator::align_cells()
{
    //join all words in a single line, aligned by base line
}


//=======================================================================================
// CellWord implementation
//=======================================================================================
void CellWord::measure(TextMeter& meter)
{
    meter.select_font(m_pStyle->get_font_name(), m_pStyle->get_font_size(),
                      m_pStyle->is_bold(), m_pStyle->is_italic() );
    m_size.height = meter.get_font_height();
    m_size.width = meter.measure_width(m_word);
    m_descent = meter.get_descender();
    m_ascent = meter.get_ascender();
}



}  //namespace lomse
