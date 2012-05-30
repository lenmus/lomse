//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_table_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// TableLayouter tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, access to protected members
class MyTableLayouter : public TableLayouter
{
protected:
//    LUnits m_firstLineIndent;
//    string m_prefix;

public:
    MyTableLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                    LibraryScope& libraryScope, ImoStyles* pStyles)
        : TableLayouter(pImo, NULL, pGModel, libraryScope, pStyles)
    {
    }
//    MyTableLayouter(LibraryScope& libraryScope, LineReferences& refs)
//        : TableLayouter(NULL, NULL, NULL, libraryScope, NULL)
//    {
//        m_lineRefs = refs;
//    }
    virtual ~MyTableLayouter() {}

    GmoBox* my_get_main_box() { return m_pItemMainBox; }
    int my_get_num_head_rows() { return m_numHeadRows; }
    int my_get_num_body_rows() { return m_numBodyRows; }
    int my_get_num_cols() { return m_numCols; }
//    std::vector<TableCellLayouter*>& my_get_body_layouters() { return m_bodyLayouters; }
//    std::vector<TableCellLayouter*>& my_get_head_layouters() { return m_headLayouters; }
//    LUnits my_get_column_width(int iCol) { return m_columnWidth[iCol]; }
//    std::vector<int>& my_get_body_row_start() { return m_bodyRowStart; }

};


//---------------------------------------------------------------------------------------
class TableLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    TableLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~TableLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

};


SUITE(TableLayouterTest)
{

    // ----------------------------------------------------------------------------------

    TEST_FIXTURE(TableLayouterTestFixture, table_box_created)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (table "
                "(tableHead (tableRow (tableCell (txt \"This is a head cell\")) ))"
                "(tableBody (tableRow (tableCell (txt \"This is a body cell\")) ))"
            ") ))");
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );

        GraphicModel model;
        GmoBoxDocPage page(NULL);
        GmoBoxDocPageContent box(NULL);
        box.set_owner_box(&page);

        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);

        GmoBox* pTableBox = lyt.my_get_main_box();
        CHECK( pTableBox != NULL );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_grid_size_1)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (table "
                "(tableBody "
                "    (tableRow (tableCell (txt \"Cell 0\"))"
                "              (tableCell (txt \"Cell 1\"))"
                "              (tableCell (txt \"Cell 2\")) )"
                "    (tableRow (tableCell (txt \"Cell 3\"))"
                "              (tableCell (txt \"Cell 4\"))"
                "              (tableCell (txt \"Cell 5\")) )"
                ")"
            ")) )");
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );

        GraphicModel model;

        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        //cout << "head rows=" << lyt.my_get_num_head_rows() << endl;
        //cout << "body rows=" << lyt.my_get_num_body_rows() << endl;
        //cout << "cols=" << lyt.my_get_num_cols() << endl;
        CHECK( lyt.my_get_num_head_rows() == 0 );
        CHECK( lyt.my_get_num_body_rows() == 2 );
        CHECK( lyt.my_get_num_cols() == 3 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_grid_size_2)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (table "
                "(tableHead "
                "    (tableRow (tableCell (txt \"Cell 1,1\"))"
                "              (tableCell (colspan 2)(txt \"Cell 1,2\")) )"
                ")"
                "(tableBody "
                "    (tableRow (tableCell (txt \"Cell 1,1\"))"
                "              (tableCell (txt \"Cell 1,2\"))"
                "              (tableCell (txt \"Cell 1,3\")) )"
                "    (tableRow (tableCell (txt \"Cell 2,1\"))"
                "              (tableCell (txt \"Cell 2,2\"))"
                "              (tableCell (txt \"Cell 2,3\")) )"
                ")"
            ")) )");
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );

        GraphicModel model;

        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

//        cout << "head rows=" << lyt.my_get_num_head_rows() << endl;
//        cout << "body rows=" << lyt.my_get_num_body_rows() << endl;
//        cout << "cols=" << lyt.my_get_num_cols() << endl;
        CHECK( lyt.my_get_num_head_rows() == 1 );
        CHECK( lyt.my_get_num_body_rows() == 2 );
        CHECK( lyt.my_get_num_cols() == 3 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_grid_size_3)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (table "
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"0\") )"
                        "(tableCell (rowspan 2)(colspan 2)(txt \"0,2,5,6\") )"
                        "(tableCell (txt \"3\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"4\") )"
                        "(tableCell (txt \"7\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = doc.get_imodoc();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_num_head_rows() == 0 );
        CHECK( lyt.my_get_num_body_rows() == 2 );
        CHECK( lyt.my_get_num_cols() == 4 );
    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_create_layouters_1)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) "
//            "(content (table "
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"0\") )"
//                        "(tableCell (rowspan 2)(colspan 2)(txt \"1+(2,5,6)\") )"
//                        "(tableCell (txt \"3\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"4\") )"
//                        "(tableCell (txt \"7\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<TableCellLayouter*>& layouters = lyt.my_get_body_layouters();
//        //cout << "size=" <<  layouters.size() << endl;
//        CHECK( layouters.size() == 8 );
//        CHECK( layouters[0] != NULL );      // cell 0
//        CHECK( layouters[1] != NULL );      // cell 1      +---+---+---+---+
//        CHECK( layouters[2] == NULL );      // -           | 0 |       | 3 |
//        CHECK( layouters[3] != NULL );      // cell 3      +---+   1   +---+
//        //                                                 | 4 |       | 7 |
//        CHECK( layouters[4] != NULL );      // cell 4      +---+---+---+---+
//        CHECK( layouters[5] == NULL );      // -
//        CHECK( layouters[6] == NULL );      // -
//        CHECK( layouters[7] != NULL );      // cell 7
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_create_layouters_2)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) "
//            "(content (table "
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"0\") )"
//                        "(tableCell (colspan 2)(txt \"1+(2)\") )"
//                        "(tableCell (txt \"3\") )"      //implicit colspan=2
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"5\") )"
//                        "(tableCell (txt \"6\") )"
//                        "(tableCell (txt \"7\") )"
//                        "(tableCell (txt \"8\") )"
//                        "(tableCell (txt \"9\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<TableCellLayouter*>& layouters = lyt.my_get_body_layouters();
//        //cout << "size=" <<  layouters.size() << endl;
//        CHECK( layouters.size() == 10 );
//        CHECK( layouters[0] != NULL );      // cell 0
//        CHECK( layouters[1] != NULL );      // cell 1      +---+---+---+---+---+
//        CHECK( layouters[2] == NULL );      // -           | 0 |   1   |   3   |
//        CHECK( layouters[3] != NULL );      // cell 3      +---+---+---+---+---+
//        CHECK( layouters[4] == NULL );      // -           | 5 | 6 | 7 | 8 | 9 +
//        //                                                 +---+---+---+---+---+
//        CHECK( layouters[5] != NULL );      // cell 5
//        CHECK( layouters[6] != NULL );      // cell 6
//        CHECK( layouters[7] != NULL );      // cell 7
//        CHECK( layouters[8] != NULL );      // cell 8
//        CHECK( layouters[9] != NULL );      // cell 9
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_determine_widths_1)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableHead (tableRow"
//                    "(tableCell (txt \"This is head cell 1\"))"
//                    "(tableCell (txt \"This is head cell 2\"))"
//                "))"
//                "(tableBody (tableRow"
//                    "(tableCell (txt \"This is body cell 1\"))"
//                    "(tableCell (txt \"This is body cell 2\"))"
//                "))"
//            ")) )");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        CHECK( lyt.my_get_num_cols() == 2 );
//        CHECK( is_equal(lyt.my_get_column_width(0), 70.0f) );
//        CHECK( is_equal(lyt.my_get_column_width(1), 80.0f) );
//    }


//    TEST_FIXTURE(TableLayouterTestFixture, table_assign_width_to_cells_1)
//    {
////        +----------+--------------------+----------+
////        |          |      Bolfgums      |   Eesh   |
////        |          +---------+----------+   data   |
////        |          | forward | backward |          |
////        +----------+---------+----------+----------+
////        | Flumbos  |   270   |   177    |          |
////        +----------+---------+----------+    37%   |
////        | Garblos  |   315   |   187    |          |
////        +----------+---------+----------+----------+
//
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            "   (defineStyle \"table1-col3\" (table-col-width 90))"
//            "   (defineStyle \"table1-col4\" (table-col-width 70))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableColumn (style \"table1-col3\"))"
//                "(tableColumn (style \"table1-col4\"))"
//                "(tableHead"
//                    "(tableRow"
//                        "(tableCell (rowspan 2) )"
//                        "(tableCell (colspan 2) (txt \"Bolfgums\") )"
//                        "(tableCell (rowspan 2) (txt \"Eesh data\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"forward\") )"
//                        "(tableCell (txt \"backward\") )"
//                    ")"
//                ")"
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"Flumbos\") )"
//                        "(tableCell (txt \"270\") )"
//                        "(tableCell (txt \"177\") )"
//                        "(tableCell (rowspan 2) (txt \"37%\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"Garblos\") )"
//                        "(tableCell (txt \"315\") )"
//                        "(tableCell (txt \"187\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<TableCellLayouter*>& layouters = lyt.my_get_head_layouters();
////        cout << "size=" <<  layouters.size() << endl;
//        CHECK( layouters.size() == 8 );
////        cout << "head cell 0 width: " << layouters[0]->get_cell_width() << endl;
////        cout << "head cell 1 width: " << layouters[1]->get_cell_width() << endl;
////        cout << "head cell 3 width: " << layouters[3]->get_cell_width() << endl;
////        cout << "head cell 5 width: " << layouters[5]->get_cell_width() << endl;
////        cout << "head cell 6 width: " << layouters[6]->get_cell_width() << endl;
//        CHECK( is_equal(layouters[0]->get_cell_width(), 70.0f) );
//        CHECK( is_equal(layouters[1]->get_cell_width(), 170.0f) );
//        CHECK( layouters[2] == NULL );
//        CHECK( is_equal(layouters[3]->get_cell_width(), 70.0f) );
//        CHECK( layouters[4] == NULL );
//        CHECK( is_equal(layouters[5]->get_cell_width(), 80.0f) );
//        CHECK( is_equal(layouters[6]->get_cell_width(), 90.0f) );
//        CHECK( layouters[7] == NULL );
//
//        layouters = lyt.my_get_body_layouters();
////        cout << "size=" <<  layouters.size() << endl;
//        CHECK( layouters.size() == 8 );
//        CHECK( is_equal(layouters[0]->get_cell_width(), 70.0f) );
//        CHECK( is_equal(layouters[1]->get_cell_width(), 80.0f) );
//        CHECK( is_equal(layouters[2]->get_cell_width(), 90.0f) );
//        CHECK( is_equal(layouters[3]->get_cell_width(), 70.0f) );
//        CHECK( is_equal(layouters[4]->get_cell_width(), 70.0f) );
//        CHECK( is_equal(layouters[5]->get_cell_width(), 80.0f) );
//        CHECK( is_equal(layouters[6]->get_cell_width(), 90.0f) );
//        CHECK( layouters[7] == NULL );
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_1)
//    {
////        +---+---+---+
////        | 0 | 1 | 2 |
////        +---+---+---+
////        | 3 | 4 | 5 |
////        +---+---+---+
////        | 6 | 7 | 8 |
////        +---+---+---+
////        | 9 |10 |11 |
////        +---+---+---+
//
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            "   (defineStyle \"table1-col3\" (table-col-width 90))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableColumn (style \"table1-col3\"))"
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 0\") )"
//                        "(tableCell (txt \"cell 1\") )"
//                        "(tableCell (txt \"cell 2\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 3\") )"
//                        "(tableCell (txt \"cell 4\") )"
//                        "(tableCell (txt \"cell 5\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 6\") )"
//                        "(tableCell (txt \"cell 7\") )"
//                        "(tableCell (txt \"cell 8\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 9\") )"
//                        "(tableCell (txt \"cell 10\") )"
//                        "(tableCell (txt \"cell 11\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<int>& rowStart = lyt.my_get_body_row_start();
////        cout << "size=" <<  rowStart.size() << endl;
//        CHECK( rowStart.size() == 4 );
//        CHECK( rowStart[0] == 0 );
//        CHECK( rowStart[1] == 1 );
//        CHECK( rowStart[2] == 2 );
//        CHECK( rowStart[3] == 3 );
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_2)
//    {
////        +---+---+---+
////        | 0 | 1 | 2 |
////        +---+---+---+
////        | 3 | 4 | 5 |
////        +---+---+   +
////        | 6 | 7 |   |
////        +---+---+---+
////        | 9 |10 |11 |
////        +---+---+---+
//
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            "   (defineStyle \"table1-col3\" (table-col-width 90))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableColumn (style \"table1-col3\"))"
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 0\") )"
//                        "(tableCell (txt \"cell 1\") )"
//                        "(tableCell (txt \"cell 2\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 3\") )"
//                        "(tableCell (txt \"cell 4\") )"
//                        "(tableCell (rowspan 2)(txt \"cell 5\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 6\") )"
//                        "(tableCell (txt \"cell 7\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 9\") )"
//                        "(tableCell (txt \"cell 10\") )"
//                        "(tableCell (txt \"cell 11\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<int>& rowStart = lyt.my_get_body_row_start();
////        cout << "size=" <<  rowStart.size() << endl;
//        CHECK( rowStart.size() == 3 );
//        CHECK( rowStart[0] == 0 );
//        CHECK( rowStart[1] == 1 );
//        CHECK( rowStart[2] == 3 );
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_3)
//    {
////        +---+---+---+
////        | 0 | 1 | 2 |
////        +---+---+---+
////        | 3 | 4 | 5 |
////        +---+---+   +
////        | 6 | 7 |   |
////        +---+   +---+
////        | 9 |   |11 |
////        +---+---+---+
//
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            "   (defineStyle \"table1-col3\" (table-col-width 90))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableColumn (style \"table1-col3\"))"
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 0\") )"
//                        "(tableCell (txt \"cell 1\") )"
//                        "(tableCell (txt \"cell 2\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 3\") )"
//                        "(tableCell (txt \"cell 4\") )"
//                        "(tableCell (rowspan 2)(txt \"cell 5\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 6\") )"
//                        "(tableCell (rowspan 2)(txt \"cell 7\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 9\") )"
//                        "(tableCell (txt \"cell 11\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<int>& rowStart = lyt.my_get_body_row_start();
////        cout << "size=" <<  rowStart.size() << endl;
//        CHECK( rowStart.size() == 2 );
//        CHECK( rowStart[0] == 0 );
//        CHECK( rowStart[1] == 1 );
//    }

//    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_4)
//    {
////        +---+---+---+
////        | 0 | 1 | 2 |
////        +---+   +   +
////        | 3 |   |   |
////        +---+---+   +
////        | 6 | 7 |   |
////        +---+---+---+
////        | 9 |10 |11 |
////        +---+---+---+
//
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0)"
//            "(styles"
//            "   (defineStyle \"table1-col1\" (table-col-width 70))"
//            "   (defineStyle \"table1-col2\" (table-col-width 80))"
//            "   (defineStyle \"table1-col3\" (table-col-width 90))"
//            ")"
//            "(content (table "
//                "(tableColumn (style \"table1-col1\"))"
//                "(tableColumn (style \"table1-col2\"))"
//                "(tableColumn (style \"table1-col3\"))"
//                "(tableBody"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 0\") )"
//                        "(tableCell (rowspan 2)(txt \"cell 1\") )"
//                        "(tableCell (rowspan 3)(txt \"cell 2\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 3\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 6\") )"
//                        "(tableCell (txt \"cell 7\") )"
//                    ")"
//                    "(tableRow"
//                        "(tableCell (txt \"cell 9\") )"
//                        "(tableCell (txt \"cell 10\") )"
//                        "(tableCell (txt \"cell 11\") )"
//                    ")"
//                ")"
//            ")"
//            "))");
//        ImoDocument* pDoc = doc.get_imodoc();
//        ImoStyles* pStyles = pDoc->get_styles();
//        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
//        GraphicModel model;
//        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
//        lyt.prepare_to_start_layout();
//
//        std::vector<int>& rowStart = lyt.my_get_body_row_start();
////        cout << "size=" <<  rowStart.size() << endl;
//        CHECK( rowStart.size() == 2 );
//        CHECK( rowStart[0] == 0 );
//        CHECK( rowStart[1] == 3 );
//    }

};


//=======================================================================================
// TableCellSizer tests
//=======================================================================================

//---------------------------------------------------------------------------------------
class MockCellLayouter : public TableCellLayouter
{
protected:
    int m_rowspan;
    int m_colspan;

public:
    MockCellLayouter(LibraryScope& libraryScope, int rowspan, int colspan)
        : TableCellLayouter(NULL, NULL, NULL, libraryScope, NULL)
        , m_rowspan(rowspan)
        , m_colspan(colspan)
    {
    }

    int get_rowspan() { return m_rowspan; }

};

//---------------------------------------------------------------------------------------
class MyTableCellSizer : public TableCellSizer
{
public:
    MyTableCellSizer(vector<TableCellLayouter*>& cells, vector<LUnits>& heights,
                     int iFirstRow, int numRows, int numCols)
        : TableCellSizer(cells, heights, iFirstRow, numRows, numCols)
    {
    }

    void my_create_rowspan_table() { create_rowspan_table(); }
    void my_compute_heights() { compute_heights(); }
    int my_get_rowspan(int iRow, int iCol) {
        return m_rowspan[iRow * m_numColumns + iCol];
    }
    LUnits my_get_height(int iRow, int iCol) {
        return m_heights[iRow * m_numColumns + iCol];
    }

};

//---------------------------------------------------------------------------------------
class TableCellSizerTestFixture
{
public:
    LibraryScope m_libraryScope;
    vector<TableCellLayouter*> m_cellLayouters;
    vector<LUnits> m_heights;

    TableCellSizerTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
    }

    ~TableCellSizerTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    void create_table_1()
    {
        //  +----+----+         +----+----+
        //  | 12 |  8 |         | 12 | 12 |
        //  +----+----+   ==>   +----+----+
        //  | 7  |  9 |         |  9 |  9 |
        //  +----+----+         +----+----+

        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );

        m_heights.push_back(12.0f);
        m_heights.push_back(8.0f);
        m_heights.push_back(7.0f);
        m_heights.push_back(9.0f);
    }

    void create_table_2()
    {
        //  +----+----+----+----+----+         +----+----+----+----+----+
        //  |    |   10    |  8 |    |         |    |   10    | 10 |    |
        //  + 22 +----+----+----+    +         + 22 +----+----+----+    +
        //  |    |  5 |  8 |    | 25 |   ==>   |    | 12 | 12 |    | 34 |
        //  +----+----+----+ 12 +    +         +----+----+----+ 24 +    +
        //  |  0 | 10 | 12 |    |    |         | 12 | 12 | 12 |    |    |
        //  +----+----+----+----+----+         +----+----+----+----+----+

        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 2,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,2) );
        m_cellLayouters.push_back( NULL );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 3,1) );

        m_cellLayouters.push_back( NULL );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 2,1) );
        m_cellLayouters.push_back( NULL );

        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( NULL );
        m_cellLayouters.push_back( NULL );

        m_heights.push_back(22.0f);
        m_heights.push_back(10.0f);
        m_heights.push_back(0.0f);
        m_heights.push_back(8.0f);
        m_heights.push_back(25.0f);

        m_heights.push_back(0.0f);
        m_heights.push_back(5.0f);
        m_heights.push_back(8.0f);
        m_heights.push_back(12.0f);
        m_heights.push_back(0.0f);

        m_heights.push_back(0.0f);
        m_heights.push_back(10.0f);
        m_heights.push_back(12.0f);
        m_heights.push_back(0.0f);
        m_heights.push_back(0.0f);
    }

    void delete_test_data()
    {
        vector<TableCellLayouter*>::iterator it;
        for (it = m_cellLayouters.begin(); it != m_cellLayouters.end(); ++it)
            delete *it;

        m_cellLayouters.clear();
        m_heights.clear();
    }

};


SUITE(TableCellSizerTest)
{

    TEST_FIXTURE(TableCellSizerTestFixture, create_rowspan_table_1)
    {
        create_table_1();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 0, 2 /*rows*/, 2 /*cols*/);
        sizer.my_create_rowspan_table();

        CHECK( sizer.my_get_rowspan(0, 0) == 0 );
        CHECK( sizer.my_get_rowspan(0, 1) == 0 );
        CHECK( sizer.my_get_rowspan(1, 0) == 0 );
        CHECK( sizer.my_get_rowspan(1, 1) == 0 );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, create_rowspan_table_2)
    {
        create_table_2();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 0, 3 /*rows*/, 5 /*cols*/);
        sizer.my_create_rowspan_table();

        CHECK( sizer.my_get_rowspan(0, 0) == 1 );
        CHECK( sizer.my_get_rowspan(0, 1) == 0 );
        CHECK( sizer.my_get_rowspan(0, 2) == 0 );
        CHECK( sizer.my_get_rowspan(0, 3) == 0 );
        CHECK( sizer.my_get_rowspan(0, 4) == 2 );

        CHECK( sizer.my_get_rowspan(1, 0) == 0 );
        CHECK( sizer.my_get_rowspan(1, 1) == 0 );
        CHECK( sizer.my_get_rowspan(1, 2) == 0 );
        CHECK( sizer.my_get_rowspan(1, 3) == 1 );
        CHECK( sizer.my_get_rowspan(1, 4) == 1 );

        CHECK( sizer.my_get_rowspan(2, 0) == 0 );
        CHECK( sizer.my_get_rowspan(2, 1) == 0 );
        CHECK( sizer.my_get_rowspan(2, 2) == 0 );
        CHECK( sizer.my_get_rowspan(2, 3) == 0 );
        CHECK( sizer.my_get_rowspan(2, 4) == 0 );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, compute_heights_1)
    {
        create_table_1();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 0, 2 /*rows*/, 2 /*cols*/);
        sizer.my_create_rowspan_table();
        sizer.my_compute_heights();

        CHECK( is_equal( sizer.my_get_height(0, 0), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(0, 1), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 0), 9.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 1), 9.0f) );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, compute_heights_2)
    {
        create_table_2();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 0, 3 /*rows*/, 5 /*cols*/);
        sizer.my_create_rowspan_table();
        sizer.my_compute_heights();

        CHECK( is_equal( sizer.my_get_height(0, 0), 10.0f) );
        CHECK( is_equal( sizer.my_get_height(0, 1), 10.0f) );
        CHECK( is_equal( sizer.my_get_height(0, 2), 10.0f) );
        CHECK( is_equal( sizer.my_get_height(0, 3), 10.0f) );
        CHECK( is_equal( sizer.my_get_height(0, 4), 10.0f) );

        CHECK( is_equal( sizer.my_get_height(1, 0), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 1), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 2), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 3), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 4), 12.0f) );

        CHECK( is_equal( sizer.my_get_height(2, 0), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(2, 1), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(2, 2), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(2, 3), 12.0f) );
        CHECK( is_equal( sizer.my_get_height(2, 4), 12.0f) );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, logical_row_1)
    {
        create_table_1();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 1, 1 /*rows*/, 2 /*cols*/);
        sizer.my_create_rowspan_table();
        sizer.my_compute_heights();

        CHECK( is_equal( sizer.my_get_height(1, 0), 9.0f) );
        CHECK( is_equal( sizer.my_get_height(1, 1), 9.0f) );

        CHECK( sizer.my_get_rowspan(0, 0) == 0 );
        CHECK( sizer.my_get_rowspan(0, 1) == 0 );

        delete_test_data();
    }

};
