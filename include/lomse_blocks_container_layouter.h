//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
class ImoContent;
class ImoContentObj;
class ImoDocument;
class ImoList;
class ImoListItem;
class ImoMultiColumn;
class ImoScorePlayer;
class ImoStyles;
class GraphicModel;
class GmoBox;
class GmoBoxDocPage;
class DocLayouter;


//----------------------------------------------------------------------------------
// ContentLayouter: layout algorithm for a collection of content items.
class ContentLayouter : public Layouter
{
protected:
    ImoContent* m_pContent;

public:
    ContentLayouter(ImoContentObj* pItem, Layouter* pParent,
                    GraphicModel* pGModel, LibraryScope& libraryScope,
                    ImoStyles* pStyles, bool fAddShapesToModel=true);
    virtual ~ContentLayouter() {}

    //implementation of Layouter virtual methods
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);

};

//----------------------------------------------------------------------------------
// MultiColumnLayouter: layout algorithm for organising content in several columns
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
                        ImoStyles* pStyles, bool fAddShapesToModel=true);
    virtual ~MultiColumnLayouter();

    //implementation of Layouter virtual methods
    void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);

protected:
    void layout_column(Layouter* pColLayouter, GmoBox* pParentBox,
                       LUnits width, UPoint pos);

};

//---------------------------------------------------------------------------------------
// BlocksContainerLayouter: layouts a blocks container item
class BlocksContainerLayouter : public Layouter
{
public:
    BlocksContainerLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                            LibraryScope& libraryScope, ImoStyles* pStyles,
                            bool fAddShapesToModel=true);
    virtual ~BlocksContainerLayouter() {}

    //generic implementation of Layouter virtual methods
    virtual void layout_in_box();
    virtual void create_main_box(GmoBox* pParentBox, UPoint pos,
                                 LUnits width, LUnits height);
};

//----------------------------------------------------------------------------------
// ListLayouter: layout algorithm for a collection of listitems.
class ListLayouter : public BlocksContainerLayouter
{
protected:
    //ImoList* m_pList;
    LUnits m_indent;

public:
    ListLayouter(ImoContentObj* pItem, Layouter* pParent, GraphicModel* pGModel,
                 LibraryScope& libraryScope, ImoStyles* pStyles,
                 bool fAddShapesToModel=true);
    virtual ~ListLayouter() {}

    //overrides
    //void layout_in_box();
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height);
};

//---------------------------------------------------------------------------------------
// ListItemLayouter: layouts a list item
class ListItemLayouter : public BlocksContainerLayouter
{
public:
    ListItemLayouter(ImoContentObj* pImo, Layouter* pParent, GraphicModel* pGModel,
                     LibraryScope& libraryScope, ImoStyles* pStyles,
                     bool fAddShapesToModel=true);
    virtual ~ListItemLayouter() {}

    bool is_first_content_item(ImoContentObj* pImo);
};


}   //namespace lomse

#endif    // __LOMSE_CONTENT_LAYOUTER_H__

