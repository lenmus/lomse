//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;

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
    void layout_in_box() override;
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;

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
    virtual void layout_in_box() override;
    virtual void create_main_box(GmoBox* pParentBox, UPoint pos,
                                 LUnits width, LUnits height) override;
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
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width, LUnits height) override;
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

