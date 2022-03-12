//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_DOCUMENT_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_layouter.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class ImoContentObj;
class ImoDocument;
class ImoStyles;
class Layouter;
class GraphicModel;
class GmoBox;
class GmoBoxDocPage;
class ScoreLayouter;

//---------------------------------------------------------------------------------------
// DocLayouter: layouts a document
class DocLayouter : public Layouter
{
protected:
    ImoDocument* m_pDoc;
    LUnits m_viewWidth;

    //for unit tests: need to access ScoreLayouter.
    Layouter* m_pScoreLayouter;

public:
    DocLayouter(Document* pDoc, LibraryScope& libraryScope, int constrains=0,
                LUnits width=0.0f);
    virtual ~DocLayouter();

    void layout_document();
    void layout_empty_document();

    //implementation of virtual methods in Layouter base class
    void layout_in_box() override {}
    void create_main_box(GmoBox* UNUSED(pParentBox), UPoint UNUSED(pos),
                         LUnits UNUSED(width), LUnits UNUSED(height)) override {}
    GmoBox* start_new_page() override;

    //only for unit tests
    ScoreLayouter* get_score_layouter();
    void save_score_layouter(Layouter* pLayouter) override;

protected:
    int layout_content();
    void fix_document_size();
    void delete_last_trial();

    GmoBoxDocPage* create_document_page();
    void assign_paper_size_to(GmoBox* pBox);
    void add_margins_to_page(GmoBoxDocPage* pPage);
    void add_headers_to_page(GmoBoxDocPage* pPage);
    void add_footers_to_page(GmoBoxDocPage* pPage);

};


}   //namespace lomse

#endif    // __LOMSE_DOCUMENT_LAYOUTER_H__

