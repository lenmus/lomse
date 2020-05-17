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
#include "lomse_inlines_container_layouter.h"
#include "lomse_engrouters.h"
#include "lomse_injectors.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_analyser.h"
#include "private/lomse_document_p.h"
#include "lomse_im_factory.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// InlinesContainerLayouter tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyInlinesContainerLayouter : public InlinesContainerLayouter
{
public:
    MyInlinesContainerLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                        LibraryScope& libraryScope, ImoStyles* pStyles)
        : InlinesContainerLayouter(pImo, nullptr, pGModel, libraryScope, pStyles)
    {
    }
    MyInlinesContainerLayouter(LibraryScope& libraryScope, LineReferences& refs)
        : InlinesContainerLayouter(nullptr, nullptr, nullptr, libraryScope, nullptr)
    {
        m_lineRefs = refs;
    }
    virtual ~MyInlinesContainerLayouter() {}

    GmoBox* my_get_main_box() { return m_pItemMainBox; }
    UPoint my_get_cursor() { return m_pageCursor; }

    bool my_is_first_line() { return is_first_line(); }
    void my_set_first_line_indent(LUnits value) { m_firstLineIndent = value; }
    void my_set_first_line_prefix(const wstring& value) { m_firstLinePrefix = value; }
    void my_set_cursor_and_available_space() { set_cursor_and_available_space(); }
    void my_page_initializations() { page_initializations(m_pItemMainBox); }
//    void my_create_engrouters() { create_engrouters(); }
    void my_add_line() { add_line(); }
    void my_add_end_margins() { add_end_margins(); }
    void my_set_box_height() { set_box_height(); }
    bool my_enough_space_in_box() { return enough_space_in_box(); }
    void my_update_line_references(LineReferences& engr, LUnits shift, bool fUpdateText) {
        update_line_references(engr, shift, fUpdateText);
    }

    void my_prepare_line() { prepare_line(); }
//    LUnits my_get_line_width() { return m_lineWidth; }
    LUnits my_get_available_space() { return m_availableSpace; }
    void my_set_line_pos_and_width() { set_line_pos_and_width(); }

};


//---------------------------------------------------------------------------------------
class InlinesContainerLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    InlinesContainerLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~InlinesContainerLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal_pos(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(InlinesContainerLayouterTest)
{

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, prepare_line_adds_indent)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        CHECK( lyt.my_is_first_line() == true );
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.my_set_cursor_and_available_space();
        lyt.my_page_initializations();
        lyt.my_set_first_line_indent(2000.0f);
        CHECK( is_equal_pos(lyt.my_get_available_space(), 10000.0f) );

        lyt.my_set_line_pos_and_width();
        CHECK( is_equal_pos(lyt.my_get_available_space(), 8000.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, prepare_line_adds_bullet)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        CHECK( lyt.my_is_first_line() == true );
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.my_set_cursor_and_available_space();
        lyt.my_page_initializations();
        lyt.my_set_first_line_indent(2000.0f);
        lyt.my_set_first_line_prefix(L"*   ");
        CHECK( is_equal_pos(lyt.my_get_available_space(), 10000.0f) );

        lyt.my_prepare_line();
        CHECK( is_equal_pos(lyt.my_get_available_space(), 4659.22f) );
//        cout << "available = " << lyt.my_get_available_space() << endl;
    }

    // para -----------------------------------------------------------------------------

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_Measure)
    {
        //The objective of this test is not to test something but to measure a
        //paragraph without margins, so that in following tests we can check
        //paragraph dimensions when adding border, margin, padding, etc.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        CHECK( lyt.my_is_first_line() == true );
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
//        cout << "word: org=(" << pWord->get_left() << ", "
//             << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 635.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 0.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_TopMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 0.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 1000.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_LeftMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 1000.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 635.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_LeftBottomMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        pParaStyle->margin_bottom(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 1000.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_FirstLineIndent)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_left(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.my_set_first_line_indent(2000.0f);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
        //cout << "box: org=(" << pParaBox->get_origin().x << ", "
        //     << pParaBox->get_origin().y << ") size=("
        //     << pParaBox->get_size().width << ", "
        //     << pParaBox->get_size().height << ")"
        //     << endl;
        //cout << "cursor=(" << lyt.my_get_cursor().x << ", "
        //     << lyt.my_get_cursor().y << ")" << endl;
        //cout << "word: org=(" << pWord->get_left() << ", "
        //     << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 635.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 3000.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 3000.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 635.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_NotEnoughSpaceInPage)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1100.0f);
        lyt.layout_in_box();

        CHECK( lyt.is_item_layouted() == false );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_EmptyParagraphHeight)
    {
        //If the paragraph is empty, it is considered to contain a line with a
        //"strut" (an invisible glyph of zero width) with the current paragraph
        //font size. Therefore, paragraph height is given by:
        //      paragrph height = line-height * font-size
        //plus the paragraph padding & margin.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->margin_top(1000.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 1635.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 1635.0f) );
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_UpdateReferences_text_larger_up)
    {
        LineReferences line(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom
        LineReferences engr(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom

        MyInlinesContainerLayouter lyt(m_libraryScope, line);
        LUnits shift = 400.0f - 500.0f;
        lyt.my_update_line_references(engr, shift, true /*text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal_pos(refs.textTop, 160.0f) );        //mean(100+100, 120)
        CHECK( is_equal_pos(refs.middleline, 375.0f) );     //mean(100+300, 350)
        CHECK( is_equal_pos(refs.baseline, 500.0f) );       //max(100+400, 500)
        CHECK( is_equal_pos(refs.textBottom, 675.0f) );     //mean(100+600, 650)
        CHECK( is_equal_pos(refs.lineHeight, 800.0f) );     //max(100+700, 770)
        CHECK( is_equal_pos(refs.supperLine, 120.0f) );     //engr.top
        CHECK( is_equal_pos(refs.subLine, 500.0f) );        //engr.base
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_UpdateReferences_text_smaller_down)
    {
        LineReferences line(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom
        LineReferences engr(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom

        MyInlinesContainerLayouter lyt(m_libraryScope, line);
        LUnits shift = 500.0f - 400.0f;
        lyt.my_update_line_references(engr, shift, true /*text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal_pos(refs.textTop, 160.0f) );        //mean(120, 100+100)
        CHECK( is_equal_pos(refs.middleline, 375.0f) );     //mean(350, 100+300)
        CHECK( is_equal_pos(refs.baseline, 500.0f) );       //max(500, 100+400)
        CHECK( is_equal_pos(refs.textBottom, 675.0f) );     //mean(650, 100+600)
        CHECK( is_equal_pos(refs.lineHeight, 800.0f) );     //max(770, 100+700)
        CHECK( is_equal_pos(refs.supperLine, 200.0f) );     //engr.top + shift
        CHECK( is_equal_pos(refs.subLine, 500.0f) );        //engr.base + shift
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_UpdateReferences_notext_larger_down)
    {
        LineReferences line(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom
        LineReferences engr(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom

        MyInlinesContainerLayouter lyt(m_libraryScope, line);
        LUnits shift = 400.0f - 350.0f;     //i.e. sub align
        lyt.my_update_line_references(engr, shift, false /*text, no_baseline / no text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal_pos(refs.textTop, 100.0f) );        //no change
        CHECK( is_equal_pos(refs.middleline, 350.0f) );     //mean(300, 350+50)
        CHECK( is_equal_pos(refs.baseline, 400.0f) );       //no change
        CHECK( is_equal_pos(refs.textBottom, 600.0f) );     //no change
        CHECK( is_equal_pos(refs.lineHeight, 820.0f) );     //max(700, 770+50)
        CHECK( is_equal_pos(refs.supperLine, 100.0f) );     //no change
        CHECK( is_equal_pos(refs.subLine, 400.0f) );        //no change
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_UpdateReferences_notext_smaller_up)
    {
        LineReferences line(120.0f, 350.0f, 500.0f, 650.0f); //top, middle, base, bottom
        LineReferences engr(100.0f, 300.0f, 400.0f, 600.0f); //top, middle, base, bottom

        MyInlinesContainerLayouter lyt(m_libraryScope, line);
        LUnits shift = 120.0f - 300.0f;     //i.e. supper align
        lyt.my_update_line_references(engr, shift, false /*text, no_baseline / no text*/);

        LineReferences& refs = lyt.get_line_refs();

//        cout << "top=" << refs.textTop << ", middle=" << refs.middleline
//             << ", base=" << refs.baseline << ", bottom=" << refs.textBottom
//             << ", height=" << refs.lineHeight << endl;

        CHECK( is_equal_pos(refs.textTop, 120.0f) );        //no change
        CHECK( is_equal_pos(refs.middleline, 415.0f) );     //mean(350+180, 300)
        CHECK( is_equal_pos(refs.baseline, 500.0f) );       //no change
        CHECK( is_equal_pos(refs.textBottom, 650.0f) );     //no change
        CHECK( is_equal_pos(refs.lineHeight, 950.0f) );     //max(770+180, 700)
        CHECK( is_equal_pos(refs.supperLine, 120.0f) );     //no change
        CHECK( is_equal_pos(refs.subLine, 500.0f) );        //no change
    }

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_AlignBaseline)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pParaStyle = doc.create_private_style();
        pParaStyle->margin(0.0f);
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        ImoStyle* pTextStyle1 = doc.create_private_style();
        pTextStyle1->margin(0.0f)->vertical_align(ImoStyle::k_valign_baseline)->font_size(12.0f);
        pPara->add_text_item("Exercise ", pTextStyle1);
        ImoStyle* pTextStyle2 = doc.create_private_style();
        pTextStyle2->margin(0.0f)->vertical_align(ImoStyle::k_valign_baseline)->font_size(14.0f);
        pPara->add_text_item("Options ", pTextStyle2);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
//        GmoShapeWord* pWord1 = static_cast<GmoShapeWord*>( pParaBox->get_shape(0) );
//        GmoShapeWord* pWord2 = static_cast<GmoShapeWord*>( pParaBox->get_shape(1) );
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "word1.top=" << pWord1->get_top() << ", baseline="
//             << pWord1->get_baseline() << endl;
//        cout << "word2.top=(" << pWord2->get_top() << ", baseline="
//             << pWord2->get_baseline() << endl;
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 740.833f) );
//TODO: REVIEW THESE
//        CHECK( pWord1->get_top() > 0.0f );
//        CHECK( is_equal_pos(pWord2->get_top(), 0.0f) );
//        CHECK( is_equal_pos(pWord1->get_baseline(), pWord2->get_baseline()) );
    }

////    //TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_TwoPagesCreated)
////    //{
////    //    Document doc(m_libraryScope);
////    //    doc.from_string("(lenmusdoc (version 0.0)(content "
////    //        "(para (text \"Line one one one one one one one one one one\")"
////    //        "      (text \"Line two two two two two two two two two two\")"
////    //        ") ))" );
////
////    //    DomunetLayouter lyt(m_libraryScope);
////    //    lyt.prepare_to_start_layout();
////    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1500.0f);
////    //    lyt.layout_in_box();
////
////    //    CHECK( lyt.is_item_layouted() == false );
////    //}

    TEST_FIXTURE(InlinesContainerLayouterTestFixture, Paragraph_MinHeight)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        pDefStyle->vertical_align(ImoStyle::k_valign_top);
        pDefStyle->min_height(1200.0f);
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_im_root()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyInlinesContainerLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        CHECK( lyt.my_is_first_line() == true );
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);
        lyt.layout_in_box();
        lyt.my_set_box_height();
        lyt.my_add_end_margins();

        GmoBox* pParaBox = lyt.my_get_main_box();
        GmoShape* pWord = pParaBox->get_shape(0);
//        cout << "box: org=(" << pParaBox->get_origin().x << ", "
//             << pParaBox->get_origin().y << ") size=("
//             << pParaBox->get_size().width << ", "
//             << pParaBox->get_size().height << ")"
//             << endl;
//        cout << "cursor=(" << lyt.my_get_cursor().x << ", "
//             << lyt.my_get_cursor().y << ")" << endl;
//        cout << "word: org=(" << pWord->get_left() << ", "
//             << pWord->get_top() << ")" << endl;
        CHECK( is_equal_pos(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal_pos(lyt.my_get_cursor().y, 635.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal_pos(pParaBox->get_size().height, 1200.0f) );
        CHECK( is_equal_pos(pWord->get_left(), 0.0f) );
        CHECK( is_equal_pos(pWord->get_top(), 0.0f) );
    }

};
