//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
//#include "lomse_layouter.h"

//#include <sstream>
//using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class GraphicModel;
class GmoBox;
class ImoStyles;

//----------------------------------------------------------------------------------
// Layouter
// Abstract class to implement the layout algorithm for any document content item.
class Layouter
{
protected:
    bool m_fIsLayouted;
    GraphicModel* m_pGModel;
    Layouter* m_pParentLayouter;
    LibraryScope& m_libraryScope;
    ImoStyles* m_pStyles;
    GmoBox* m_pItemMainBox;     //this layouter main box
    Layouter* m_pCurLayouter;
    ImoContentObj* m_pItem;

    //position (relative to page origin) and available space in current box
    LUnits m_availableWidth;
    LUnits m_availableHeight;
    UPoint m_pageCursor;

    //constructor
    Layouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
             LibraryScope& libraryScope, ImoStyles* pStyles);
    //constructor for DocumentLayouter
    Layouter(LibraryScope& libraryScope);

    //info
    inline GmoBox* get_item_main_box() { return m_pItemMainBox; }

public:
    virtual ~Layouter() {}

    virtual void layout_in_box() = 0;
    virtual void prepare_to_start_layout() { m_fIsLayouted = false; }
    virtual bool is_item_layouted() { return m_fIsLayouted; }
    virtual void set_layout_is_finished(bool value) { m_fIsLayouted = value; }
    virtual void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                                 LUnits height) = 0;
    virtual void save_score_layouter(Layouter* pLayouter) {
        m_pParentLayouter->save_score_layouter(pLayouter);
    }

    inline GraphicModel* get_graphic_model() { return m_pGModel; }
    inline LibraryScope& get_library_scope() { return m_libraryScope; }


protected:
    virtual GmoBox* start_new_page();

    Layouter* create_layouter(ImoContentObj* pItem);
    void layout_item(ImoContentObj* pItem, GmoBox* pParentBox);

    void add_end_margins();
    void set_box_height();
    void set_cursor_and_available_space(GmoBox* pItemMainBox);

    inline UPoint get_cursor() { return m_pageCursor; }
    inline LUnits get_available_width() { return m_availableWidth; }
    inline LUnits get_available_height() { return m_availableHeight; }
};


//----------------------------------------------------------------------------------
class LayouterFactory
{
public:
    LayouterFactory() {}
    ~LayouterFactory() {}

    static Layouter* create_layouter(ImoContentObj* pImo, Layouter* pParent,
                                     ImoStyles* pStyles);
};


//----------------------------------------------------------------------------------
class NullLayouter : public Layouter
{
protected:

public:
    NullLayouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
                 LibraryScope& libraryScope)
        : Layouter(pItem, pParent, pGModel, libraryScope, NULL)
    {
    }
    ~NullLayouter() {}

    void layout_in_box() {}
    bool is_item_layouted() { return true; }
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                         LUnits height) {}
};


}   //namespace lomse

#endif    // __LOMSE_LAYOUTER_H__

