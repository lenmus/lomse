//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_blocks_container_layouter.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyListLayouter : public ListLayouter
{
public:
    MyListLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                   LibraryScope& libraryScope, ImoStyles* pStyles)
        : ListLayouter(pImo, nullptr, pGModel, libraryScope, pStyles)
    {
    }
    virtual ~MyListLayouter() {}

    GmoBox* my_get_main_box() { return m_pItemMainBox; }
    UPoint my_get_cursor() { return m_pageCursor; }
    void my_add_end_margins() { add_end_margins(); }
    void my_set_box_height() { set_box_height(); }

};


//---------------------------------------------------------------------------------------
class ListLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ListLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ListLayouterTestFixture()  // tearDown()
    {
    }

};


SUITE(ListLayouterTest)
{

    // para -----------------------------------------------------------------------------

//    TEST_FIXTURE(ListLayouterTestFixture, List_Measure)
//    {
//        //The objective of this test is not to test something but to measure a
//        //paragraph without margins, so that in following tests we can check
//        //list and paragraph dimensions.
//
//        Document doc(m_libraryScope);
//        doc.create_empty();
//        ImoStyle* pDefStyle = doc.get_default_style();
//        pDefStyle->vertical_align(ImoStyle::k_valign_top);
//        ImoStyle* pListStyle = doc.create_style("list");
//        ImoList* pList = doc.get_im_root()->get_content()->add_list(ImoList::k_itemized, pListStyle);
//        ImoListItem* pListitem = pList->add_listitem(pDefStyle);
//        ImoTextItem* pText = static_cast<ImoTextItem*>(
//                                    ImFactory::inject(k_imo_text_item, &doc) );
//        pText->set_text("Exercise options");
//        pText->set_style(pDefStyle);
//        pListitem->append_content_item(pText);
//
//        GraphicModel model;
//        ImoDocument* pDoc = doc.get_im_root();
//        ImoStyles* pStyles = pDoc->get_styles();
//        GmoBoxDocPage page(nullptr);
//        GmoBoxDocPageContent box(nullptr);
//        box.set_owner_box(&page);
//
//        MyListLayouter lyt(pList, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
//        lyt.layout_in_box();
//        lyt.my_set_box_height();
//        lyt.my_add_end_margins();
//
//        GmoBox* pListBox = lyt.my_get_main_box();
//        GmoBox* pListItemBox = pListBox->get_child_box(0);
//        GmoBox* pParaBox = pListItemBox->get_child_box(0);
//        CHECK( pParaBox != nullptr );
//        GmoShape* pWord = pParaBox->get_shape(0);
//        CHECK( pWord != nullptr );
////        cout << "box: org=(" << pListBox->get_origin().x << ", "
////             << pListBox->get_origin().y << ") size=("
////             << pListBox->get_size().width << ", "
////             << pListBox->get_size().height << ")"
////             << endl;
////        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
////             << lyt.my_get_cursor().y << ")" << endl;
////        cout << "word: org=(" << pWord->get_left() << ", "
////             << pWord->get_top() << ")" << endl;
//        CHECK( is_equal_pos(lyt.my_get_cursor().x, 1000.0f) );
//        CHECK( is_equal_pos(lyt.my_get_cursor().y, 635.0f) );
//        CHECK( is_equal_pos(pListBox->get_origin().x, 1000.0f) );
//        CHECK( is_equal_pos(pListBox->get_origin().y, 0.0f) );
//        CHECK( is_equal_pos(pListBox->get_size().width, 9000.0f) );
//        CHECK( is_equal_pos(pListBox->get_size().height, 635.0f) );
//        CHECK( is_equal_pos(pWord->get_left(), 500.0f) );
//        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
//    }

};
