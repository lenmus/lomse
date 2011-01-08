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

#ifndef __LOMSE_DOCUMENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_DOCUMENT_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class GraphicModel;
class ImoDocObj;
class ContentLayouter;
class FlowSizer;
class ImoDocument;
class GmoBox;
class GmoBoxDocPage;
class FontStorage;


// DocLayouter: layouts a document
//---------------------------------------------------------------------------------------
class DocLayouter
{
protected:
    InternalModel* m_pIModel;
    GraphicModel* m_pGModel;
    LibraryScope& m_libraryScope;
    //FlowSizer* m_pMainSizer;
    GmoBox* m_pCurrentBox;
    LUnits m_availableWidth;
    LUnits m_availableHeight;
    UPoint m_pageCursor;            //to point to current position. Relative to BoxDocPage


public:
    DocLayouter(InternalModel* pIModel, LibraryScope& libraryScope);
    virtual ~DocLayouter();

    void layout_document();
    inline GraphicModel* get_gm_model() { return m_pGModel; }


protected:
    void initializations();
    void layout_content();
    void layout_item(GmoBox* pParentBox, ImoDocObj* pItem);
    //void add_content_to_main_sizer();
    //void assign_space_to_content_items();
    ImoDocument* get_document();
    ContentLayouter* new_item_layouter(ImoDocObj* pImo);
    void start_new_document_page();
    GmoBoxDocPage* create_document_page();
    void assign_paper_size_to(GmoBox* pBox);
    void add_margins_to_page(GmoBoxDocPage* pPage);
    void add_headers_to_page(GmoBoxDocPage* pPage);
    void add_footers_to_page(GmoBoxDocPage* pPage);
    void add_contents_wrapper_box_to_page(GmoBoxDocPage* pPage);
    GmoBox* create_item_pagebox(GmoBox* pParentBox, ContentLayouter* pLayouter);

};


}   //namespace lomse

#endif    // __LOMSE_DOCUMENT_LAYOUTER_H__

