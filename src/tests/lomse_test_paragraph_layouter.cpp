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
#include "lomse_config.h"

//classes related to these tests
#include "lomse_paragraph_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_analyser.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// CellsCreator tests
//=======================================================================================
//---------------------------------------------------------------------------------------
// helper, to access protected members
class MyCellsCreator : public CellsCreator
{
public:
    MyCellsCreator(ImoParagraph* pPara, std::list<Cell*>& cells,
                   LibraryScope& libraryScope)
        : CellsCreator(pPara, cells, libraryScope)
    {
    }
    virtual ~MyCellsCreator() {}

    void my_create_text_item_cells(ImoTextItem* pText) { create_text_item_cells(pText); }
    void my_measure_cells() { measure_cells(); }
    void my_align_cells() { align_cells(); }

};
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

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_InitialSpaces)
    {
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( f.create_object(
            "(txt \" This is a paragraph\")"));
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( pCell->get_text() == " " );

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_FirstWordAfterInitialSpaces)
    {
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( f.create_object(
            "(txt \" This is a paragraph\")"));
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
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

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_SpacesCompressed)
    {
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( f.create_object(
            "(txt \"  This   is   a   paragraph\")"));
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
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

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_OneWord)
    {
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( f.create_object(
            "(txt \"Hello!\")"));
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( cells.size() == 1 );
        CHECK( pCell->get_text() == "Hello!" );

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, SplitTextItem_OneWordSpaces)
    {
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( f.create_object(
            "(txt \"Hello!   \")"));
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( cells.size() == 1 );
        CHECK( pCell->get_text() == "Hello! " );

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CellWordHasStyle)
    {
        ImoTextStyleInfo style;
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = f.create_text_item("(txt \"Hello!\")", &style);
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        CHECK( pCell->get_style() != NULL );

        delete pText;
    }

    TEST_FIXTURE(CellsCreatorTestFixture, CellWordMeasurements)
    {
        ImoTextStyleInfo style;
        ImoObjFactory f(m_libraryScope, cout);
        ImoTextItem* pText = f.create_text_item("(txt \"Hello!\")", &style);
        std::list<Cell*> cells;
        MyCellsCreator creator(NULL, cells, m_libraryScope);
        creator.my_create_text_item_cells(pText);
        creator.my_measure_cells();

        CellWord *pCell = dynamic_cast<CellWord*>( cells.front() );
        cout << "ascent = " << pCell->get_ascent() << endl;
        cout << "descent = " << pCell->get_descent() << endl;
        cout << "height = " << pCell->get_height() << endl;

        delete pText;
    }

};



//=======================================================================================
// ParagraphLayouter tests
//=======================================================================================

////---------------------------------------------------------------------------------------
//// helper, access to protected members
class MyParagraphLayouter : public ParagraphLayouter
{
//protected:
//    LibraryScope& m_libraryScope;
//    ImoParagraph* m_pPara;
//    ImoStyles* m_pStyles;
//    GmoBox* m_pMainBox;
//    UPoint m_cursor;                //current position. Relative to BoxDocPage
//
//    int m_iCurLine;         //index to first line not yet included in paragraph

public:
    MyParagraphLayouter(ImoContentObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope,
                      ImoStyles* pStyles)
        : ParagraphLayouter(pImo, pGModel, libraryScope, pStyles)
    {
    }
    virtual ~MyParagraphLayouter() {}

    GmoBox* my_get_main_box() { return m_pMainBox; }
    UPoint my_get_cursor() { return m_cursor; }

    void my_page_initializations(GmoBox* pMainBox) { page_initializations(pMainBox); }
    void my_create_cells() { create_cells(); }
    void my_add_line() { add_line(); }

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

    TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_CreateLayouter)
    {
        ImoParagraph para;
        ImoStyles styles;
        GraphicModel model;
        MyParagraphLayouter lyt(&para, &model, m_libraryScope, &styles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_main_box() != NULL );
        CHECK( is_equal(lyt.my_get_cursor().x, 0.0f) );
        CHECK( is_equal(lyt.my_get_cursor().y, 0.0f) );
    }

    //TEST_FIXTURE(ParagraphLayouterTestFixture, Paragraph_LayoutInPage)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content "
    //        "(para (txt \"Hello world!\")) ))");
    //    InternalModel* pIModel = doc.get_im_model();
    //    ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
    //    ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
    //    MyDocLayouter dl(pIModel, m_libraryScope);
    //    dl.my_initializations();
    //    ParagraphLayouter* pLyt
    //        = dynamic_cast<ParagraphLayouter*>( dl.my_new_item_layouter(pPara) );
    //    GmoBox* pBox = dl.my_get_current_box();
    //
    //    pLyt->layout_in_page(pBox);

    //    CHECK( pLyt != NULL );

    //    delete pLyt;
    //}

};
