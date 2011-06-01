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

#ifndef __LOMSE_TEXT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_TEXT_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_drawer.h"
#include "lomse_content_layouter.h"
#include <sstream>

using namespace std;

namespace lomse
{

//forward declarations
class ImoDocObj;
class ImoParagraph;
class GraphicModel;
class GmoBox;
//class InternalModel;
//class GmoBoxDocPage;
//class FontStorage;
//class FlowSizer;
//class ImoDocument;


// ParagraphLayouter: layouts a paragraph
//---------------------------------------------------------------------------------------
class ParagraphLayouter : public ContentLayouter
{
protected:
    LibraryScope& m_libraryScope;
    ImoParagraph* m_pPara;
    //InternalModel* m_pIModel;
    //GraphicModel* m_pGModel;
    ////FlowSizer* m_pMainSizer;
    //GmoBox* m_pCurrentBox;
    //LUnits m_availableWidth;
    //LUnits m_availableHeight;
    //UPoint m_pageCursor;            //to point to current position. Relative to BoxDocPage


public:
    ParagraphLayouter(ImoDocObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope);
    virtual ~ParagraphLayouter();

    //virtual methods in base class
    void layout_in_page(GmoBox* pContainerBox);
    GmoBox* create_main_box();

protected:
    void add_text_item();

};


}   //namespace lomse

#endif    // __LOMSE_TEXT_LAYOUTER_H__

