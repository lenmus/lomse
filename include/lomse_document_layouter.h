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
#include "lomse_layouter.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
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

    //for unit tests: need to access ScoreLayouter.
    Layouter* m_pScoreLayouter;

public:
    DocLayouter(InternalModel* pIModel, LibraryScope& libraryScope);
    virtual ~DocLayouter();

    void layout_document();

    //implementation of virtual methods in Layouter base class
    void layout_in_box() {}
    void create_main_box(GmoBox* pParentBox, UPoint pos, LUnits width,
                         LUnits height) {}
    GmoBox* start_new_page();

    //only for unit tests
    ScoreLayouter* get_score_layouter();
    void save_score_layouter(Layouter* pLayouter);
    inline GraphicModel* get_gm_model() { return m_pGModel; }

protected:
    void layout_content();

    GmoBoxDocPage* create_document_page();
    void assign_paper_size_to(GmoBox* pBox);
    void add_margins_to_page(GmoBoxDocPage* pPage);
    void add_headers_to_page(GmoBoxDocPage* pPage);
    void add_footers_to_page(GmoBoxDocPage* pPage);

};


}   //namespace lomse

#endif    // __LOMSE_DOCUMENT_LAYOUTER_H__

