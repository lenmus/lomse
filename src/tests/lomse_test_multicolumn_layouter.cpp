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
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// MultiColumnLayouter tests
//=======================================================================================

////---------------------------------------------------------------------------------------
//// helper, access to protected members
//class MyMultiColumnLayouter : public MultiColumnLayouter
//{
//public:
//    MyMultiColumnLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
//                        LibraryScope& libraryScope, ImoStyles* pStyles)
//        : MultiColumnLayouter(pImo, nullptr, pGModel, libraryScope, pStyles)
//    {
//    }
//    virtual ~MyMultiColumnLayouter() {}
//
//    GmoBox* my_get_main_box() { return m_pItemMainBox; }
//    UPoint my_get_cursor() { return m_pageCursor; }
//
//    void my_page_initializations(GmoBox* pMainBox) { page_initializations(pMainBox); }
//    void my_create_cells() { create_cells(); }
//    void my_add_line() { add_line(); }
//    void my_add_end_margins() { add_end_margins(); }
//    void my_set_box_height() { set_box_height(); }
//    bool my_enough_space_in_box() { return enough_space_in_box(); }
//
//};


//---------------------------------------------------------------------------------------
class MultiColumnLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MultiColumnLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MultiColumnLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(MultiColumnLayouterTest)
{

    TEST_FIXTURE(MultiColumnLayouterTestFixture, Paragraph_OnlyText)
    {
        //Document doc(m_libraryScope);
        //doc.create_empty();
        //ImoStyle* pDefStyle = doc.get_default_style();
        //ImoStyle* pParaStyle = doc.create_style("para");
        //ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        //pPara->add_text_item("Exercise options", pDefStyle);

        //GraphicModel model;
        //ImoDocument* pDoc = doc.get_im_root();
        //ImoStyles* pStyles = pDoc->get_styles();
        //GmoBoxDocPage page(nullptr);
        //GmoBoxDocPageContent box(nullptr);
        //box.set_owner_box(&page);

        //MyMultiColumnLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        //lyt.prepare_to_start_layout();
        //lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        //lyt.layout_in_box();
        //lyt.my_set_box_height();
        //lyt.my_add_end_margins();

        //GmoBox* pParaBox = lyt.my_get_main_box();
        //GmoShape* pWord = pParaBox->get_shape(0);
        ////cout << "box: org=(" << pParaBox->get_origin().x << ", "
        ////     << pParaBox->get_origin().y << ") size=("
        ////     << pParaBox->get_size().width << ", "
        ////     << pParaBox->get_size().height << ")"
        ////     << endl;
        ////cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        ////     << lyt.my_get_cursor().y << ")" << endl;
        ////cout << "word: org=(" << pWord->get_left() << ", "
        ////     << pWord->get_top() << ")" << endl;
        //CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        //CHECK( is_equal(lyt.my_get_cursor().y, 476.25f) );
        //CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        //CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        //CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        //CHECK( is_equal(pParaBox->get_size().height, 476.25f) );
        //CHECK( is_equal(pWord->get_left(), 0.0f) );
        //CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
    }

    //TEST_FIXTURE(MultiColumnLayouterTestFixture, Paragraph_TopMargin)
    //{
    //    Document doc(m_libraryScope);
    //    doc.create_empty();
    //    ImoStyle* pDefStyle = doc.get_default_style();
    //    ImoStyle* pParaStyle = doc.create_style("para");
    //    pParaStyle->margin_top( 1000.0f);
    //    ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
    //    pPara->add_text_item("Exercise options", pDefStyle);

    //    GraphicModel model;
    //    ImoDocument* pDoc = doc.get_im_root();
    //    ImoStyles* pStyles = pDoc->get_styles();
    //    GmoBoxDocPage page(nullptr);
    //    GmoBoxDocPageContent box(nullptr);
    //    box.set_owner_box(&page);

    //    MyMultiColumnLayouter lyt(pPara, &model, m_libraryScope, pStyles);
    //    lyt.prepare_to_start_layout();
    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
    //    lyt.layout_in_box();
    //    lyt.my_set_box_height();
    //    lyt.my_add_end_margins();

    //    GmoBox* pParaBox = lyt.my_get_main_box();
    //    GmoShape* pWord = pParaBox->get_shape(0);
    //    //cout << "box: org=(" << pParaBox->get_origin().x << ", "
    //    //     << pParaBox->get_origin().y << ") size=("
    //    //     << pParaBox->get_size().width << ", "
    //    //     << pParaBox->get_size().height << ")"
    //    //     << endl;
    //    //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
    //    //     << lyt.my_get_cursor().y << ")" << endl;
    //    //cout << "word: org=(" << pWord->get_left() << ", "
    //    //     << pWord->get_top() << ")" << endl;
    //    CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
    //    CHECK( is_equal(pParaBox->get_size().height, 1476.25f) );
    //    CHECK( is_equal(pWord->get_left(), 0.0f) );
    //    CHECK( is_equal(pWord->get_top(), 1011.73f) );        //shift 11.729
    //    CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
    //    CHECK( is_equal(lyt.my_get_cursor().y, 1476.25f) );
    //}

    //TEST_FIXTURE(MultiColumnLayouterTestFixture, Paragraph_LeftMargin)
    //{
    //    Document doc(m_libraryScope);
    //    doc.create_empty();
    //    ImoStyle* pDefStyle = doc.get_default_style();
    //    ImoStyle* pParaStyle = doc.create_style("para");
    //    pParaStyle->margin_left( 1000.0f);
    //    ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
    //    pPara->add_text_item("Exercise options", pDefStyle);

    //    GraphicModel model;
    //    ImoDocument* pDoc = doc.get_im_root();
    //    ImoStyles* pStyles = pDoc->get_styles();
    //    GmoBoxDocPage page(nullptr);
    //    GmoBoxDocPageContent box(nullptr);
    //    box.set_owner_box(&page);

    //    MyMultiColumnLayouter lyt(pPara, &model, m_libraryScope, pStyles);
    //    lyt.prepare_to_start_layout();
    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
    //    lyt.layout_in_box();
    //    lyt.my_set_box_height();
    //    lyt.my_add_end_margins();

    //    GmoBox* pParaBox = lyt.my_get_main_box();
    //    GmoShape* pWord = pParaBox->get_shape(0);
    //    //cout << "box: org=(" << pParaBox->get_origin().x << ", "
    //    //     << pParaBox->get_origin().y << ") size=("
    //    //     << pParaBox->get_size().width << ", "
    //    //     << pParaBox->get_size().height << ")"
    //    //     << endl;
    //    //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
    //    //     << lyt.my_get_cursor().y << ")" << endl;
    //    //cout << "word: org=(" << pWord->get_left() << ", "
    //    //     << pWord->get_top() << ")" << endl;
    //    CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
    //    CHECK( is_equal(pParaBox->get_size().height, 476.25f) );
    //    CHECK( is_equal(pWord->get_left(), 1000.0f) );
    //    CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
    //    CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
    //    CHECK( is_equal(lyt.my_get_cursor().y, 476.25f) );
    //}

    //TEST_FIXTURE(MultiColumnLayouterTestFixture, Paragraph_LeftBottomMargin)
    //{
    //    Document doc(m_libraryScope);
    //    doc.create_empty();
    //    ImoStyle* pDefStyle = doc.get_default_style();
    //    ImoStyle* pParaStyle = doc.create_style("para");
    //    pParaStyle->margin_left( 1000.0f);
    //    pParaStyle->margin_bottom( 1000.0f);
    //    ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
    //    pPara->add_text_item("Exercise options", pDefStyle);

    //    GraphicModel model;
    //    ImoDocument* pDoc = doc.get_im_root();
    //    ImoStyles* pStyles = pDoc->get_styles();
    //    GmoBoxDocPage page(nullptr);
    //    GmoBoxDocPageContent box(nullptr);
    //    box.set_owner_box(&page);

    //    MyMultiColumnLayouter lyt(pPara, &model, m_libraryScope, pStyles);
    //    lyt.prepare_to_start_layout();
    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
    //    lyt.layout_in_box();
    //    lyt.my_set_box_height();
    //    lyt.my_add_end_margins();

    //    GmoBox* pParaBox = lyt.my_get_main_box();
    //    GmoShape* pWord = pParaBox->get_shape(0);
    //    //cout << "box: org=(" << pParaBox->get_origin().x << ", "
    //    //     << pParaBox->get_origin().y << ") size=("
    //    //     << pParaBox->get_size().width << ", "
    //    //     << pParaBox->get_size().height << ")"
    //    //     << endl;
    //    //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
    //    //     << lyt.my_get_cursor().y << ")" << endl;
    //    //cout << "word: org=(" << pWord->get_left() << ", "
    //    //     << pWord->get_top() << ")" << endl;
    //    CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
    //    CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
    //    CHECK( is_equal(pParaBox->get_size().height, 1476.25f) );
    //    CHECK( is_equal(pWord->get_left(), 1000.0f) );
    //    CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
    //    CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
    //    CHECK( is_equal(lyt.my_get_cursor().y, 1476.25f) );
    //}

    //TEST_FIXTURE(MultiColumnLayouterTestFixture, Paragraph_NotEnoughSpaceInPage)
    //{
    //    Document doc(m_libraryScope);
    //    doc.create_empty();
    //    ImoStyle* pDefStyle = doc.get_default_style();
    //    ImoStyle* pParaStyle = doc.create_style("para");
    //    pParaStyle->margin_top( 1000.0f);
    //    ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
    //    pPara->add_text_item("Exercise options", pDefStyle);

    //    GraphicModel model;
    //    ImoDocument* pDoc = doc.get_im_root();
    //    ImoStyles* pStyles = pDoc->get_styles();
    //    GmoBoxDocPage page(nullptr);
    //    GmoBoxDocPageContent box(nullptr);
    //    box.set_owner_box(&page);

    //    MyMultiColumnLayouter lyt(pPara, &model, m_libraryScope, pStyles);
    //    lyt.prepare_to_start_layout();
    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1300.0f);
    //    lyt.layout_in_box();

    //    CHECK( lyt.is_item_layouted() == false );
    //}

};
