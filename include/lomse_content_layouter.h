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

#ifndef __LOMSE_CONTENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_CONTENT_LAYOUTER_H__

#include "lomse_layouter.h"
#include "lomse_injectors.h"
#include "lomse_basic.h"

#include <vector>
using namespace std;


namespace lomse
{

//forward declarations
class InternalModel;
//class ImoControl;
class ImoContent;
class ImoContentObj;
class ImoDocument;
class ImoMultiColumn;
class ImoStyles;
class GraphicModel;
class GmoBox;
class GmoBoxDocPage;
class DocLayouter;


//----------------------------------------------------------------------------------
// ContentLayouter
//  layout algorithm for a collection of content items.
class ContentLayouter : public Layouter
{
protected:
    ImoContent* m_pContent;

public:
    ContentLayouter(ImoContentObj* pItem, Layouter* pParent,
                    GraphicModel* pGModel, LibraryScope& libraryScope, ImoStyles* pStyles);
    virtual ~ContentLayouter() {}

    //implementation of Layouter virtual methods
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);

};


//----------------------------------------------------------------------------------
// MultiColumnLayouter
//  layout algorithm for organising content in several columns
class MultiColumnLayouter : public Layouter
{
protected:
    ImoMultiColumn* m_pMultiColumn;
    vector<Layouter*> m_colLayouters;
    vector<UPoint> m_colPosition;
    vector<LUnits> m_colWidth;
    UPoint m_cursor;

public:
    MultiColumnLayouter(ImoContentObj* pItem, Layouter* pParent,
                        GraphicModel* pGModel, LibraryScope& libraryScope,
                        ImoStyles* pStyles);
    virtual ~MultiColumnLayouter();

    //implementation of Layouter virtual methods
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);

protected:
    void layout_column(Layouter* pColLayouter, GmoBox* pParentBox,
                       LUnits width, UPoint pos);

};


////----------------------------------------------------------------------------------
//// ControlLayouter
////  layout algorithm for a gui control
//class ControlLayouter : public Layouter
//{
//protected:
//    ImoControl* m_pControl;
//
//public:
//    ControlLayouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
//                  LibraryScope& libraryScope, ImoStyles* pStyles);
//    virtual ~ControlLayouter() {}
//
//    //implementation of Layouter virtual methods
//    void layout_in_box();
//    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);
//
//};


}   //namespace lomse

#endif    // __LOMSE_CONTENT_LAYOUTER_H__

