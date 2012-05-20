//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

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
    inline ImoStyles* get_styles() { return m_pStyles; }

    void add_end_margins();
    LUnits set_box_height();
    inline GmoBox* get_item_main_box() { return m_pItemMainBox; }

protected:
    virtual GmoBox* start_new_page();

    Layouter* create_layouter(ImoContentObj* pItem);
    void layout_item(ImoContentObj* pItem, GmoBox* pParentBox);

    void set_cursor_and_available_space();

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

    static Layouter* create_layouter(ImoContentObj* pImo, Layouter* pParent);
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

