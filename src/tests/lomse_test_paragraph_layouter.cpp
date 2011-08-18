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
#include "lomse_paragraph_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;

    #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new DEBUG_NEW



//=======================================================================================
// CellsCreator tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, to access protected members
class MyCellsCreator : public CellsCreator
{
public:
    MyCellsCreator(std::list<Cell*>& cells, LibraryScope& libraryScope)
        : CellsCreator(cells, libraryScope)
    {
    }
    virtual ~MyCellsCreator() {}

    void my_create_text_item_cells(ImoTextItem* pText) { create_text_item_cells(pText); }
    void my_measure_cells() { measure_cells(); }

    void my_delete_cells() {
        std::list<Cell*>::iterator it;
        for (it = m_cells.begin(); it != m_cells.end(); ++it)
            delete *it;
    }

};

//---------------------------------------------------------------------------------------
class CellsCreatorTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    CellsCreatorTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~CellsCreatorTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(CellsCreatorTest)
{

    TEST_FIXTURE(CellsCreatorTestFixture, CreateCells_AtomicCell)
    {
        Document doc(m_libraryScope);
        ImoButton* pImo = static_cast<ImoButton*>(
                                ImFactory::inject(k_imo_button, &doc) );
        pImo->set_label("Click me!");
        pImo->set_size(USize(2000.0f, 600.0f));
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);

        CHECK( cells.size() == 1 );
        CellButton* pCell = dynamic_cast<CellButton*>( cells.front() );
        CHECK( pCell != NULL );

        creator.my_delete_cells();
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, WrapperCellCreatesBox)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);

        CHECK( cells.size() == 1 );
        CellBox* pCell = dynamic_cast<CellBox*>( cells.front() );
        CHECK( pCell != NULL );

        creator.my_delete_cells();
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, WrapperCellContentAddedToBox)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt2);

        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);

        CHECK( cells.size() == 1 );
        CellBox* pCell = dynamic_cast<CellBox*>( cells.front() );
        CHECK( pCell != NULL );

        std::list<Cell*>& children = pCell->get_cells();
        std::list<Cell*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( dynamic_cast<CellButton*>(*it) != NULL );
        ++it;
        CHECK( dynamic_cast<CellButton*>(*it) != NULL );

        creator.my_delete_cells();
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, WrapperCellBoxHasSize_Linear)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt2);

        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);

        CHECK( cells.size() == 1 );
        CellBox* pCell = dynamic_cast<CellBox*>( cells.front() );
        CHECK( pCell != NULL );
        CHECK( pCell->get_width() == 4000.0f );
        CHECK( pCell->get_height() == 600.0f );
        std::list<Cell*>& children = pCell->get_cells();
        std::list<Cell*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( (*it)->get_position() == UPoint(0.0f, 0.0f) );
        ++it;
        CHECK( (*it)->get_position() == UPoint(2000.0f, 0.0f) );

        creator.my_delete_cells();
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, WrapperCellBoxHasSize_Constrained)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        pImo->set_width(3000.0f);
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt2);

        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);

        CHECK( cells.size() == 1 );
        CellBox* pCell = dynamic_cast<CellBox*>( cells.front() );
        CHECK( pCell != NULL );
        CHECK( pCell->get_width() == 3000.0f );
        CHECK( pCell->get_height() == 1200.0f );
        std::list<Cell*>& children = pCell->get_cells();
        std::list<Cell*>::iterator it = children.begin();
        CHECK( children.size() == 2 );
        CHECK( (*it)->get_position() == UPoint(0.0f, 0.0f) );
        ++it;
        CHECK( (*it)->get_position() == UPoint(0.0f, 600.0f) );

        creator.my_delete_cells();
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_InitialSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text(" This is a paragraph");
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( pCell->get_text() == " " );

        creator.my_delete_cells();
        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_FirstWordAfterInitialSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text(" This is a paragraph");
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CHECK( cells.size() == 5 );
        std::list<Cell*>::iterator it = cells.begin();
        CellWord* pCell = dynamic_cast<CellWord*>(*it);
        CHECK( pCell->get_text() == " " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "This " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "is " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "a " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "paragraph" );

        creator.my_delete_cells();
        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_SpacesCompressed)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("  This   is   a   paragraph");
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CHECK( cells.size() == 5 );
        std::list<Cell*>::iterator it = cells.begin();
        CellWord* pCell = dynamic_cast<CellWord*>(*it);
        CHECK( pCell->get_text() == " " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "This " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "is " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "a " );
        pCell = dynamic_cast<CellWord*>(*(++it));
        CHECK( pCell->get_text() == "paragraph" );

        creator.my_delete_cells();
        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_OneWord)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( cells.size() == 1 );
        CHECK( pCell->get_text() == "Hello!" );

        creator.my_delete_cells();
        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_OneWordSpaces)
    {
        Document doc(m_libraryScope);
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!   ");
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( cells.size() == 1 );
        CHECK( pCell->get_text() == "Hello! " );

        creator.my_delete_cells();
        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CellWordHasStyle)
    {
        Document doc(m_libraryScope);
        ImoStyle* pStyle = static_cast<ImoStyle*>(
                                ImFactory::inject(k_imo_style, &doc) );
        pStyle->set_name("test");
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        pText->set_style(pStyle);
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( pCell->get_style() != NULL );

        creator.my_delete_cells();
        delete pText;
        delete pStyle;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CellWordMeasurements)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pStyle = doc.get_default_style();
        ImoTextItem* pText = static_cast<ImoTextItem*>(
                                ImFactory::inject(k_imo_text_item, &doc) );
        pText->set_text("Hello!");
        pText->set_style(pStyle);
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);
        creator.my_measure_cells();

//        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
//        cout << "ascent = " << pCell->get_ascent() << endl;
//        cout << "descent = " << pCell->get_descent() << endl;
//        cout << "height = " << pCell->get_height() << endl;

        creator.my_delete_cells();
        delete pText;
    }

    // shapes creation ------------------------------------------------------------------

    TEST_FIXTURE(CellsCreatorTestFixture, CreateGmo_AtomicCell)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoButton* pImo = static_cast<ImoButton*>(
                                ImFactory::inject(k_imo_button, &doc) );
        pImo->set_label("Click me!");
        pImo->set_size(USize(2000.0f, 600.0f));
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);
        Cell* pCell = cells.front();
        UPoint pos(1500.0f, 2700.0f);
        ImoStyle* pStyle = doc.get_default_style();
        pImo->set_style(pStyle);
        GmoObj* pGmo = pCell->create_gm_object(pos, 800.0f);    //(800-600)/2 = 100 valign

        CHECK( pGmo != NULL );
        CHECK( pGmo->is_shape_button() == true );
        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 100.0f) );

        creator.my_delete_cells();
        delete pGmo;
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CreateGmo_BoxNoContent)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        USize size(500.0f, 600.0f);
        pImo->set_size(size);
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);
        Cell* pCell = cells.front();
        UPoint pos(1500.0f, 2700.0f);
        //ImoStyle style("Test", NULL);
        //pImo->set_style(&style);
        GmoObj* pGmo = pCell->create_gm_object(pos, 800.0f);    //(800-0)/2 = 400 valign

        //cout << "box size = (" << pGmo->get_size().width << ", "
        //     << pGmo->get_size().height << ")" << endl;
        //cout << "box pos = (" << pGmo->get_origin().x << ", "
        //     << pGmo->get_origin().y << ")" << endl;
        CHECK( pGmo != NULL );
        CHECK( pGmo->is_box_inline() == true );
        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 400.0f) );
        CHECK( pGmo->get_size() == USize(500.0f, 0.0f) );

        creator.my_delete_cells();
        delete pGmo;
        delete pImo;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CreateGmo_BoxContent)
    {
        Document doc(m_libraryScope);
        ImoInlineWrapper* pImo = static_cast<ImoInlineWrapper*>(
                                    ImFactory::inject(k_imo_inline_wrapper, &doc) );
        ImoButton* pBt1 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt1->set_label("Accept");
        pBt1->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt1);
        ImoButton* pBt2 = static_cast<ImoButton*>(
                                    ImFactory::inject(k_imo_button, &doc));
        pBt2->set_label("Cancel");
        pBt2->set_size(USize(2000.0f, 600.0f));
        pImo->append_child(pBt2);

        doc.create_empty();
        ImoStyle* pStyle = doc.get_default_style();
        pBt1->set_style(pStyle);
        pBt2->set_style(pStyle);
        std::list<Cell*> cells;
        MyCellsCreator creator(cells, m_libraryScope);
        creator.create_cells(pImo);
        Cell* pCell = cells.front();
        UPoint pos(1500.0f, 2700.0f);
        GmoObj* pGmo = pCell->create_gm_object(pos, 800.0f);    //(800-600)/2 = 100 valign

        //cout << "box size = (" << pGmo->get_size().width << ", "
        //     << pGmo->get_size().height << ")" << endl;
        //cout << "box pos = (" << pGmo->get_origin().x << ", "
        //     << pGmo->get_origin().y << ")" << endl;
        CHECK( pGmo != NULL );
        CHECK( pGmo->is_box_inline() == true );
        CHECK( pGmo->get_origin() == UPoint(1500.0f, 2700.0f + 100.0f) );
        CHECK( pGmo->get_size() == USize(4000.0f, 600.0f) );

        creator.my_delete_cells();
        delete pGmo;
        delete pImo;
    }


};



//=======================================================================================
// ParagraphLayouter tests
//=======================================================================================

////---------------------------------------------------------------------------------------
//// helper, access to protected members
class MyParagraphLayouter : public ParagraphLayouter
{
public:
    MyParagraphLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                        LibraryScope& libraryScope, ImoStyles* pStyles)
        : ParagraphLayouter(pImo, NULL, pGModel, libraryScope, pStyles)
    {
    }
    virtual ~MyParagraphLayouter() {}

    GmoBox* my_get_main_box() { return m_pItemMainBox; }
    UPoint my_get_cursor() { return m_pageCursor; }

    void my_page_initializations(GmoBox* pMainBox) { page_initializations(pMainBox); }
    void my_create_cells() { create_cells(); }
    void my_add_line() { add_line(); }
    void my_add_end_margins() { add_end_margins(); }
    void my_set_box_height() { set_box_height(); }
    bool my_enough_space_in_box() { return enough_space_in_box(); }

};


//---------------------------------------------------------------------------------------
class ParagraphLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ParagraphLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~ParagraphLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(ParagraphLayouterTest)
{

    // para -----------------------------------------------------------------------------

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_OnlyText)
    {
        //The objective is to measure a paragraph without margins, so that
        //in following tests we can check paragraph dimensions when adding
        //border, margin, padding, etc.

        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        ImoStyle* pParaStyle = doc.create_style("para");
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyParagraphLayouter lyt(pPara, &model, m_libraryScope, pStyles);
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
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 476.25f) );
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 476.25f) );
        CHECK( is_equal(pWord->get_left(), 0.0f) );
        CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
    }

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_TopMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->set_lunits_property(ImoStyle::k_margin_top, 1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyParagraphLayouter lyt(pPara, &model, m_libraryScope, pStyles);
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
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 1476.25f) );
        CHECK( is_equal(pWord->get_left(), 0.0f) );
        CHECK( is_equal(pWord->get_top(), 1011.73f) );        //shift 11.729
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 1476.25f) );
    }

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_LeftMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->set_lunits_property(ImoStyle::k_margin_left, 1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyParagraphLayouter lyt(pPara, &model, m_libraryScope, pStyles);
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
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 476.25f) );
        CHECK( is_equal(pWord->get_left(), 1000.0f) );
        CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
        CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 476.25f) );
    }

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_LeftBottomMargin)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->set_lunits_property(ImoStyle::k_margin_left, 1000.0f);
        pParaStyle->set_lunits_property(ImoStyle::k_margin_bottom, 1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyParagraphLayouter lyt(pPara, &model, m_libraryScope, pStyles);
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
        CHECK( is_equal(pParaBox->get_origin().x, 0.0f) );
        CHECK( is_equal(pParaBox->get_origin().y, 0.0f) );
        CHECK( is_equal(pParaBox->get_size().width, 10000.0f) );
        CHECK( is_equal(pParaBox->get_size().height, 1476.25f) );
        CHECK( is_equal(pWord->get_left(), 1000.0f) );
        CHECK( is_equal(pWord->get_top(), 11.73f) );        //shift 11.729
        CHECK( is_equal(lyt.my_get_cursor().x, 1000.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 1476.25f) );
    }

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_NotEnoughSpaceInPage)
    {
        Document doc(m_libraryScope);
        doc.create_empty();
        ImoStyle* pDefStyle = doc.get_default_style();
        ImoStyle* pParaStyle = doc.create_style("para");
        pParaStyle->set_lunits_property(ImoStyle::k_margin_top, 1000.0f);
        ImoParagraph* pPara = doc.get_imodoc()->get_content()->add_paragraph(pParaStyle);
        pPara->add_text_item("Exercise options", pDefStyle);

        GraphicModel model;
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyParagraphLayouter lyt(pPara, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1300.0f);
        lyt.layout_in_box();

        CHECK( lyt.is_item_layouted() == false );
    }

    //TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_TwoPagesCreated)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (version 0.0)(content "
    //        "(para (text \"Line one one one one one one one one one one\")"
    //        "      (text \"Line two two two two two two two two two two\")"
    //        ") ))" );

    //    DomunetLayouter lyt(m_libraryScope);
    //    lyt.prepare_to_start_layout();
    //    lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 1500.0f);
    //    lyt.layout_in_box();

    //    CHECK( lyt.is_item_layouted() == false );
    //}

};
