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
    LUnits m_pageWidth;
    LUnits m_pageHeight;

    //for unit tests: need to access ScoreLayouter.
    Layouter* m_pScoreLayouter;

public:
    DocLayouter(Document* pDoc, LibraryScope& libraryScope, int constrains=0);
    virtual ~DocLayouter();

    void layout_document();
    void layout_empty_document();

    //implementation of virtual methods in Layouter base class
    void layout_in_box() {}
    void create_main_box(GmoBox* UNUSED(pParentBox), UPoint UNUSED(pos),
                         LUnits UNUSED(width), LUnits UNUSED(height)) {}
    GmoBox* start_new_page();

    //only for unit tests
    ScoreLayouter* get_score_layouter();
    void save_score_layouter(Layouter* pLayouter);

protected:
    void layout_content();
    void fix_document_size();

    GmoBoxDocPage* create_document_page();
    void assign_paper_size_to(GmoBox* pBox);
    void add_margins_to_page(GmoBoxDocPage* pPage);
    void add_headers_to_page(GmoBoxDocPage* pPage);
    void add_footers_to_page(GmoBoxDocPage* pPage);

};


}   //namespace lomse

#endif    // __LOMSE_DOCUMENT_LAYOUTER_H__

