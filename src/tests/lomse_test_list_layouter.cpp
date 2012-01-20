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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_content_layouter.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_document.h"

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
        : ListLayouter(pImo, NULL, pGModel, libraryScope, pStyles)
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
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~ListLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(ListLayouterTest)
{

    // para -----------------------------------------------------------------------------

    TEST_FIXTURE(ListLayouterTestFixture, List_Measure)
    {
        //The objective of this test is not to test something but to measure a
        //paragraph without margins, so that in following tests we can check
        //list and paragraph dimensions.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pListStyle = doc.create_style("list");
        ImoList* pList = doc.get_imodoc()->get_content()->add_list(ImoList::k_itemized, pListStyle);
        ImoListItem* pListitem = pList->add_listitem(pDefStyle);
        pListitem->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyListLayouter lyt(pList, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pListBox = lyt.my_get_main_box();
        GmoBox* pParaBox = pListBox->get_child_box(0);
        CHECK( pParaBox != NULL );
        GmoShape* pWord = pParaBox->get_shape(0);
        CHECK( pWord != NULL );
//        cout << "box: org=(" << pListBox->get_origin().x << ", "
//             << pListBox->get_origin().y << ") size=("
//             << pListBox->get_size().width << ", "
//             << pListBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
//        cout << "word: org=(" << pWord->get_left() << ", "
//             << pWord->get_top() << ")" << endl;
        CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 635.0f) );
        CHECK( is_equal(pListBox->get_origin().x, 1000.0f) );
        CHECK( is_equal(pListBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pListBox->get_size().width, 9000.0f) );
        CHECK( is_equal(pListBox->get_size().height, 635.0f) );
        CHECK( is_equal(pWord->get_left(), 1000.0f) );
        CHECK( is_equal(pWord->get_top(), 0.0f) );
    }

};
