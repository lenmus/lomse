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

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_table_layouter.h"
#include "lomse_injectors.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_ldp_analyser.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"
#include "lomse_calligrapher.h"
#include "lomse_shape_text.h"

//#include "lomse_box_system.h"
//#include "lomse_shape_staff.h"
//#include "lomse_im_note.h"
#include "lomse_interactor.h"
#include "lomse_graphic_view.h"
#include "lomse_doorway.h"
//#include "lomse_screen_drawer.h"
//#include "lomse_model_builder.h"
//#include "lomse_im_factory.h"

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
        : TableLayouter(pImo, nullptr, pGModel, libraryScope, pStyles, true)
    {
    }
//    MyTableLayouter(LibraryScope& libraryScope, LineReferences& refs)
//        : TableLayouter(nullptr, nullptr, nullptr, libraryScope, nullptr)
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
    LUnits my_get_column_width(int iCol) { return m_columnsWidth[iCol]; }
    LUnits my_get_table_width() { return m_tableWidth; }
    TableSectionLayouter* my_get_head_layouter() { return m_headLayouter; }
    TableSectionLayouter* my_get_body_layouter() { return m_bodyLayouter; }

};

//---------------------------------------------------------------------------------------
// helper, to test graphic model
class MyDoorway2 : public LomseDoorway
{
protected:
    bool m_fUpdateWindowInvoked;
    bool m_fSetWindowTitleInvoked;
    std::string m_title;
    RenderingBuffer m_buffer;

public:
    MyDoorway2()
        : LomseDoorway()
    {
        init_library(k_pix_format_rgba32, 96, false);
    }
    virtual ~MyDoorway2() {}

    //void request_window_update() { m_fUpdateWindowInvoked = true; }
    void set_window_title(const std::string& title) {
        m_fSetWindowTitleInvoked = true;
        m_title = title;
    }
    void force_redraw() {}
    RenderingBuffer& get_window_buffer() { return m_buffer; }

    bool set_window_title_invoked() { return m_fSetWindowTitleInvoked; }
    bool update_window_invoked() { return m_fUpdateWindowInvoked; }
    const std::string& get_title() { return m_title; }
    double get_screen_ppi() const { return 96.0; }

};


//---------------------------------------------------------------------------------------
class TableLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    TableLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TableLayouterTestFixture()  // tearDown()
    {
    }

    void create_doc_0(SpDocument spDoc)
    {
        //     5000  4000
        //    +-----+-----+
        //    |  0  |  1  |
        //    +-----+-----+

        spDoc->from_string(
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle>"
                        "<name>table1-col1</name>"
                        "<table-col-width>5000</table-col-width>"
                    "</defineStyle>"
                    "<defineStyle>"
                        "<name>table1-col2</name>"
                        "<table-col-width>4000</table-col-width>"
                    "</defineStyle>"
                "</styles>"
                "<content>"
                    "<table>"
                        "<tableColumn style='table1-col1' />"
                        "<tableColumn style='table1-col2' />"
                        "<tableBody>"
                            "<tableRow>"
                                "<tableCell>That</tableCell>"
                                "<tableCell>This is a cell</tableCell>"
                            "</tableRow>"
                        "</tableBody>"
                    "</table>"
                "</content>"
            "</lenmusdoc>", Document::k_format_lmd);
    }

    void create_doc_1(SpDocument spDoc)
    {
        //     50  40  25  15
        //    +---+---+---+---+
        //    | 0 |       | 3 |
        //    +---+   1   +---+
        //    | 4 |       | 7 |
        //    +---+---+---+---+

        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(styles"
                "(defineStyle \"table1-col1\" (table-col-width 5000))"
                "(defineStyle \"table1-col2\" (table-col-width 4000))"
                "(defineStyle \"table1-col3\" (table-col-width 2500))"
                "(defineStyle \"table1-col4\" (table-col-width 1500))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableColumn (style \"table1-col4\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"0\") )"
                        "(tableCell (rowspan 2)(colspan 2)(txt \"1+(2,5,6)\") )"
                        "(tableCell (txt \"3\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"4\") )"
                        "(tableCell (txt \"7\") )"
                    ")"
                ")"
            ")"
            "))");
    }

    void create_doc_2(SpDocument spDoc)
    {
        //   50  40  25  15  20
        //  +---+---+---+---+---+
        //  | 0 |   1   |   3   |
        //  +---+---+---+---+---+
        //  | 5 | 6 | 7 | 8 | 9 +
        //  +---+---+---+---+---+

        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(styles"
                "(defineStyle \"table1-col1\" (table-col-width 5000))"
                "(defineStyle \"table1-col2\" (table-col-width 4000))"
                "(defineStyle \"table1-col3\" (table-col-width 2500))"
                "(defineStyle \"table1-col4\" (table-col-width 1500))"
                "(defineStyle \"table1-col5\" (table-col-width 2000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableColumn (style \"table1-col4\"))"
                "(tableColumn (style \"table1-col5\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"0\") )"
                        "(tableCell (colspan 2)(txt \"1+(2)\") )"
                        "(tableCell (txt \"3\") )"      //implicit colspan=2
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"5\") )"
                        "(tableCell (txt \"6\") )"
                        "(tableCell (txt \"7\") )"
                        "(tableCell (txt \"8\") )"
                        "(tableCell (txt \"9\") )"
                    ")"
                ")"
            ")"
            "))");
    }

    void create_doc_3(SpDocument spDoc)
    {
        //      7000      8000      9000        7000
        //  +----------+--------------------+----------+
        //  |          |      Bolfgums      |   Eesh   |
        //  |          +---------+----------+   data   |
        //  |          | forward | backward |          |
        //  +----------+---------+----------+----------+
        //  | Flumbos  |   270   |   177    |          |
        //  +----------+---------+----------+    37%   |
        //  | Garblos  |   315   |   187    |          |
        //  +----------+---------+----------+----------+

        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 7000))"
            "   (defineStyle \"table1-col2\" (table-col-width 8000))"
            "   (defineStyle \"table1-col3\" (table-col-width 9000))"
            "   (defineStyle \"table1-col4\" (table-col-width 7000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableColumn (style \"table1-col4\"))"
                "(tableHead"
                    "(tableRow"
                        "(tableCell (rowspan 2) )"
                        "(tableCell (colspan 2) (txt \"Bolfgums\") )"
                        "(tableCell (rowspan 2) (txt \"Eesh data\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"forward\") )"
                        "(tableCell (txt \"backward\") )"
                    ")"
                ")"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"Flumbos\") )"
                        "(tableCell (txt \"270\") )"
                        "(tableCell (txt \"177\") )"
                        "(tableCell (rowspan 2) (txt \"37%\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"Garblos\") )"
                        "(tableCell (txt \"315\") )"
                        "(tableCell (txt \"187\") )"
                    ")"
                ")"
            ")"
            "))");
    }

};


SUITE(TableLayouterTest)
{

    // ----------------------------------------------------------------------------------

    TEST_FIXTURE(TableLayouterTestFixture, table_box_created)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (table "
                "(tableHead (tableRow (tableCell (txt \"This is a head cell\")) ))"
                "(tableBody (tableRow (tableCell (txt \"This is a body cell\")) ))"
            ") ))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );

        GraphicModel model;
        GmoBoxDocPage page(nullptr);
        GmoBoxDocPageContent box(nullptr);
        box.set_owner_box(&page);

        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();
        lyt.create_main_box(&box, UPoint(0.0f, 0.0f), 10000.0f, 20000.0f);

        GmoBox* pTableBox = lyt.my_get_main_box();
        CHECK( pTableBox != nullptr );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_grid_size_1)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
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
        ImoDocument* pDoc = spDoc->get_im_root();
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
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
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
        ImoDocument* pDoc = spDoc->get_im_root();
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
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
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
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_num_head_rows() == 0 );
        CHECK( lyt.my_get_num_body_rows() == 2 );
        CHECK( lyt.my_get_num_cols() == 4 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_widths_1)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string("(lenmusdoc (vers 0.0) "
            "(styles"
                "(defineStyle \"table1-col1\" (table-col-width 5000))"
                "(defineStyle \"table1-col2\" (table-col-width 4000))"
                "(defineStyle \"table1-col3\" (table-col-width 2500))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"0\") )"
                        "(tableCell (txt \"1+(2,5,6)\") )"
                        "(tableCell (txt \"3\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"4\") )"
                        "(tableCell (txt \"7\") )"
                        "(tableCell (txt \"8\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_num_cols() == 3 );
        CHECK( lyt.my_get_column_width(0) == 5000.0f );
        CHECK( lyt.my_get_column_width(1) == 4000.0f );
        CHECK( lyt.my_get_column_width(2) == 2500.0f );
        CHECK( lyt.my_get_table_width() == 11500.0f );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_widths_2)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        create_doc_1(spDoc);
        //    +---+---+---+---+
        //    | 0 |       | 3 |
        //    +---+   1   +---+
        //    | 4 |       | 7 |
        //    +---+---+---+---+
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_num_cols() == 4 );
        CHECK( lyt.my_get_column_width(0) == 5000.0f );
        CHECK( lyt.my_get_column_width(1) == 4000.0f );
        CHECK( lyt.my_get_column_width(2) == 2500.0f );
        CHECK( lyt.my_get_column_width(3) == 1500.0f );
        CHECK( lyt.my_get_table_width() == 13000.0f );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_determine_widths_3)
    {
        //       7000   8000
        //    +-------+-------+
        //    |   0H  |   1H  |
        //    +=======+=======+
        //    |   0B  |   1B  |
        //    +-------+-------+
        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 7000))"
            "   (defineStyle \"table1-col2\" (table-col-width 8000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableHead (tableRow"
                    "(tableCell (txt \"This is head cell 1\"))"
                    "(tableCell (txt \"This is head cell 2\"))"
                "))"
                "(tableBody (tableRow"
                    "(tableCell (txt \"This is body cell 1\"))"
                    "(tableCell (txt \"This is body cell 2\"))"
                "))"
            ")) )");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_num_cols() == 2 );
        CHECK( is_equal_pos(lyt.my_get_column_width(0), 7000.0f) );
        CHECK( is_equal_pos(lyt.my_get_column_width(1), 8000.0f) );
        CHECK( is_equal_pos(lyt.my_get_table_width(), 15000.0f) );
    }

    TEST_FIXTURE(TableLayouterTestFixture, body_layouter_created_1)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        create_doc_1(spDoc);
        //    +---+---+---+---+
        //    | 0 |       | 3 |
        //    +---+   1   +---+
        //    | 4 |       | 7 |
        //    +---+---+---+---+
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_head_layouter() == nullptr );
        TableSectionLayouter* pSL = lyt.my_get_body_layouter();
        CHECK( pSL != nullptr );
    }

    TEST_FIXTURE(TableLayouterTestFixture, create_cell_layouters_1)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        create_doc_1(spDoc);
        //    +---+---+---+---+
        //    | 0 |       | 3 |
        //    +---+   1   +---+
        //    | 4 |       | 7 |
        //    +---+---+---+---+
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_head_layouter() == nullptr );
        TableSectionLayouter* pSL = lyt.my_get_body_layouter();
        CHECK( pSL != nullptr );
        vector<TableCellLayouter*>& cellLyt = pSL->dbg_get_cell_layouters();
        CHECK( cellLyt.size() == 8 );
                                                            //   50  40  25  15
        CHECK( cellLyt[0]->get_cell_width() == 5000.0f );   //  +---+---+---+---+
        CHECK( cellLyt[1]->get_cell_width() == 6500.0f );   //  | 0 |       | 3 |
        CHECK( cellLyt[2] == nullptr );                        //  +---+   1   +---+
        CHECK( cellLyt[3]->get_cell_width() == 1500.0f );   //  | 4 |       | 7 |
        CHECK( cellLyt[4]->get_cell_width() == 5000.0f );   //  +---+---+---+---+
        CHECK( cellLyt[5] == nullptr );
        CHECK( cellLyt[6] == nullptr );
        CHECK( cellLyt[7]->get_cell_width() == 1500.0f );
    }

    TEST_FIXTURE(TableLayouterTestFixture, create_cell_layouters_2)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        create_doc_2(spDoc);
            //   50  40  25  15  20
            //  +---+---+---+---+---+
            //  | 0 |   1   |   3   |
            //  +---+---+---+---+---+
            //  | 5 | 6 | 7 | 8 | 9 +
            //  +---+---+---+---+---+
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        CHECK( lyt.my_get_head_layouter() == nullptr );
        TableSectionLayouter* pSL = lyt.my_get_body_layouter();
        CHECK( pSL != nullptr );
        vector<TableCellLayouter*>& cellLyt = pSL->dbg_get_cell_layouters();
        CHECK( cellLyt.size() == 10 );

        CHECK( cellLyt[0]->get_cell_width() == 5000.0f );   //   50  40  25  15  20
        CHECK( cellLyt[1]->get_cell_width() == 6500.0f );   //  +---+---+---+---+---+
        CHECK( cellLyt[2] == nullptr );                        //  | 0 |   1   |   3   |
        CHECK( cellLyt[3]->get_cell_width() == 3500.0f );   //  +---+---+---+---+---+
        CHECK( cellLyt[4] == nullptr );                        //  | 5 | 6 | 7 | 8 | 9 +
        //                                                  //  +---+---+---+---+---+
        CHECK( cellLyt[5]->get_cell_width() == 5000.0f );
        CHECK( cellLyt[6]->get_cell_width() == 4000.0f );
        CHECK( cellLyt[7]->get_cell_width() == 2500.0f );
        CHECK( cellLyt[8]->get_cell_width() == 1500.0f );
        CHECK( cellLyt[9]->get_cell_width() == 2000.0f );
    }

    TEST_FIXTURE(TableLayouterTestFixture, create_cell_layouters_3)
    {
        SpDocument spDoc( new Document(m_libraryScope) );
        create_doc_3(spDoc);
        //      7000      8000      9000        7000
        //  +----------+--------------------+----------+
        //  |          |      Bolfgums      |   Eesh   |
        //  |          +---------+----------+   data   |
        //  |          | forward | backward |          |
        //  +----------+---------+----------+----------+
        //  | Flumbos  |   270   |   177    |          |
        //  +----------+---------+----------+    37%   |
        //  | Garblos  |   315   |   187    |          |
        //  +----------+---------+----------+----------+
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        TableSectionLayouter* pSH = lyt.my_get_head_layouter();
        CHECK( pSH != nullptr );
        vector<TableCellLayouter*>& cellLytH = pSH->dbg_get_cell_layouters();
        CHECK( cellLytH.size() == 8 );

        CHECK( is_equal_pos(cellLytH[0]->get_cell_width(), 7000.0f) );
        CHECK( is_equal_pos(cellLytH[1]->get_cell_width(), 17000.0f) );
        CHECK( cellLytH[2] == nullptr );
        CHECK( is_equal_pos(cellLytH[3]->get_cell_width(), 7000.0f) );

        CHECK( cellLytH[4] == nullptr );
        CHECK( is_equal_pos(cellLytH[5]->get_cell_width(), 8000.0f) );
        CHECK( is_equal_pos(cellLytH[6]->get_cell_width(), 9000.0f) );
        CHECK( cellLytH[7] == nullptr );

        TableSectionLayouter* pSB = lyt.my_get_body_layouter();
        CHECK( pSB != nullptr );
        vector<TableCellLayouter*>& cellLytB = pSB->dbg_get_cell_layouters();
        CHECK( cellLytB.size() == 8 );

        CHECK( is_equal_pos(cellLytB[0]->get_cell_width(), 7000.0f) );
        CHECK( is_equal_pos(cellLytB[1]->get_cell_width(), 8000.0f) );
        CHECK( is_equal_pos(cellLytB[2]->get_cell_width(), 9000.0f) );
        CHECK( is_equal_pos(cellLytB[3]->get_cell_width(), 7000.0f) );

        CHECK( is_equal_pos(cellLytB[4]->get_cell_width(), 7000.0f) );
        CHECK( is_equal_pos(cellLytB[5]->get_cell_width(), 8000.0f) );
        CHECK( is_equal_pos(cellLytB[6]->get_cell_width(), 9000.0f) );
        CHECK( cellLytB[7] == nullptr );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_1)
    {
//        +---+---+---+
//        | 0 | 1 | 2 |
//        +---+---+---+
//        | 3 | 4 | 5 |
//        +---+---+---+
//        | 6 | 7 | 8 |
//        +---+---+---+
//        | 9 |10 |11 |
//        +---+---+---+

        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 7000))"
            "   (defineStyle \"table1-col2\" (table-col-width 8000))"
            "   (defineStyle \"table1-col3\" (table-col-width 9000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"cell 0\") )"
                        "(tableCell (txt \"cell 1\") )"
                        "(tableCell (txt \"cell 2\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 3\") )"
                        "(tableCell (txt \"cell 4\") )"
                        "(tableCell (txt \"cell 5\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 6\") )"
                        "(tableCell (txt \"cell 7\") )"
                        "(tableCell (txt \"cell 8\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 9\") )"
                        "(tableCell (txt \"cell 10\") )"
                        "(tableCell (txt \"cell 11\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        TableSectionLayouter* pSH = lyt.my_get_head_layouter();
        CHECK( pSH == nullptr );

        TableSectionLayouter* pSB = lyt.my_get_body_layouter();
        CHECK( pSB != nullptr );
        vector<int>& rowStart = pSB->dbg_get_row_start();
//        cout << "size=" <<  rowStart.size() << endl;
        CHECK( rowStart.size() == 4 );
        CHECK( rowStart[0] == 0 );
        CHECK( rowStart[1] == 1 );
        CHECK( rowStart[2] == 2 );
        CHECK( rowStart[3] == 3 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_2)
    {
//        +---+---+---+
//        | 0 | 1 | 2 |
//        +---+---+---+
//        | 3 | 4 | 5 |
//        +---+---+   +
//        | 6 | 7 |   |
//        +---+---+---+
//        | 9 |10 |11 |
//        +---+---+---+

        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 7000))"
            "   (defineStyle \"table1-col2\" (table-col-width 8000))"
            "   (defineStyle \"table1-col3\" (table-col-width 9000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"cell 0\") )"
                        "(tableCell (txt \"cell 1\") )"
                        "(tableCell (txt \"cell 2\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 3\") )"
                        "(tableCell (txt \"cell 4\") )"
                        "(tableCell (rowspan 2)(txt \"cell 5\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 6\") )"
                        "(tableCell (txt \"cell 7\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 9\") )"
                        "(tableCell (txt \"cell 10\") )"
                        "(tableCell (txt \"cell 11\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        TableSectionLayouter* pSH = lyt.my_get_head_layouter();
        CHECK( pSH == nullptr );

        TableSectionLayouter* pSB = lyt.my_get_body_layouter();
        CHECK( pSB != nullptr );
        vector<int>& rowStart = pSB->dbg_get_row_start();
//        cout << "size=" <<  rowStart.size() << endl;
        CHECK( rowStart.size() == 3 );
        CHECK( rowStart[0] == 0 );
        CHECK( rowStart[1] == 1 );
        CHECK( rowStart[2] == 3 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_3)
    {
//        +---+---+---+
//        | 0 | 1 | 2 |
//        +---+---+---+
//        | 3 | 4 | 5 |
//        +---+---+   +
//        | 6 | 7 |   |
//        +---+   +---+
//        | 9 |   |11 |
//        +---+---+---+

        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 7000))"
            "   (defineStyle \"table1-col2\" (table-col-width 8000))"
            "   (defineStyle \"table1-col3\" (table-col-width 9000))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"cell 0\") )"
                        "(tableCell (txt \"cell 1\") )"
                        "(tableCell (txt \"cell 2\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 3\") )"
                        "(tableCell (txt \"cell 4\") )"
                        "(tableCell (rowspan 2)(txt \"cell 5\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 6\") )"
                        "(tableCell (rowspan 2)(txt \"cell 7\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 9\") )"
                        "(tableCell (txt \"cell 11\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        TableSectionLayouter* pSH = lyt.my_get_head_layouter();
        CHECK( pSH == nullptr );

        TableSectionLayouter* pSB = lyt.my_get_body_layouter();
        CHECK( pSB != nullptr );
        vector<int>& rowStart = pSB->dbg_get_row_start();
//        cout << "size=" <<  rowStart.size() << endl;
        CHECK( rowStart.size() == 2 );
        CHECK( rowStart[0] == 0 );
        CHECK( rowStart[1] == 1 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, table_logical_rows_4)
    {
//        +---+---+---+
//        | 0 | 1 | 2 |
//        +---+   +   +
//        | 3 |   |   |
//        +---+---+   +
//        | 6 | 7 |   |
//        +---+---+---+
//        | 9 |10 |11 |
//        +---+---+---+

        SpDocument spDoc( new Document(m_libraryScope) );
        spDoc->from_string(
            "(lenmusdoc (vers 0.0)"
            "(styles"
            "   (defineStyle \"table1-col1\" (table-col-width 70))"
            "   (defineStyle \"table1-col2\" (table-col-width 80))"
            "   (defineStyle \"table1-col3\" (table-col-width 90))"
            ")"
            "(content (table "
                "(tableColumn (style \"table1-col1\"))"
                "(tableColumn (style \"table1-col2\"))"
                "(tableColumn (style \"table1-col3\"))"
                "(tableBody"
                    "(tableRow"
                        "(tableCell (txt \"cell 0\") )"
                        "(tableCell (rowspan 2)(txt \"cell 1\") )"
                        "(tableCell (rowspan 3)(txt \"cell 2\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 3\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 6\") )"
                        "(tableCell (txt \"cell 7\") )"
                    ")"
                    "(tableRow"
                        "(tableCell (txt \"cell 9\") )"
                        "(tableCell (txt \"cell 10\") )"
                        "(tableCell (txt \"cell 11\") )"
                    ")"
                ")"
            ")"
            "))");
        ImoDocument* pDoc = spDoc->get_im_root();
        ImoStyles* pStyles = pDoc->get_styles();
        ImoTable* pTable = static_cast<ImoTable*>( pDoc->get_content_item(0) );
        GraphicModel model;
        MyTableLayouter lyt(pTable, &model, m_libraryScope, pStyles);
        lyt.prepare_to_start_layout();

        TableSectionLayouter* pSH = lyt.my_get_head_layouter();
        CHECK( pSH == nullptr );

        TableSectionLayouter* pSB = lyt.my_get_body_layouter();
        CHECK( pSB != nullptr );
        vector<int>& rowStart = pSB->dbg_get_row_start();
//        cout << "size=" <<  rowStart.size() << endl;
        CHECK( rowStart.size() == 2 );
        CHECK( rowStart[0] == 0 );
        CHECK( rowStart[1] == 3 );
    }

    TEST_FIXTURE(TableLayouterTestFixture, gmodel_0)
    {
        MyDoorway2 doorway;
        LibraryScope libraryScope(cout, &doorway);
        libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        SpDocument spDoc( new Document(libraryScope) );
        create_doc_0(spDoc);
        //     5000  4000
        //    +-----+-----+
        //    |  0  |  1  |
        //    +-----+-----+

        VerticalBookView* pView = dynamic_cast<VerticalBookView*>(
            Injector::inject_View(libraryScope, k_view_vertical_book, spDoc.get()) );
        Interactor* pIntor = Injector::inject_Interactor(libraryScope, spDoc, pView, nullptr);
        GraphicModel* pModel = pIntor->get_graphic_model();

        CHECK( pModel != nullptr );

        //GmoBoxDocPage* pPage = pModel->get_page(0);     //DocPage
        //GmoBox* pBDPC = pPage->get_child_box(0);        //DocPageContent
        //GmoBox* pBSP = pBDPC->get_child_box(0);         //ScorePage
        //GmoBox* pBSys = pBSP->get_child_box(0);         //System
        //GmoBox* pBSlice = pBSys->get_child_box(0);          //Slice
        //GmoBox* pBSliceInstr = pBSlice->get_child_box(0);   //SliceInsr
        //LUnits x = pBSliceInstr->get_left() + 1.0f;
        //LUnits y = pBSliceInstr->get_top() + 1.0f;

        ////cout << "DocPage: " << pPage->get_left() << ", " << pPage->get_top() << endl;
        ////cout << "DocPageContent: " << pBDPC->get_left() << ", " << pBDPC->get_top() << endl;
        ////cout << "ScorePage: " << pBSP->get_left() << ", " << pBSP->get_top() << endl;
        ////cout << "System: " << pBSys->get_left() << ", " << pBSys->get_top() << endl;
        ////cout << "Slice: " << pBSlice->get_left() << ", " << pBSlice->get_top() << endl;
        ////cout << "SliceInsr: " << pBSliceInstr->get_left() << ", " << pBSliceInstr->get_top() << endl;
        ////cout << "Finding: " << x << ", " << y << endl;

        //GmoBox* pHit = pPage->find_inner_box_at(x, y);

        //CHECK ( pHit != nullptr );
        //CHECK ( pHit == pBSliceInstr );

        delete pIntor;
    }

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
        : TableCellLayouter(nullptr, nullptr, nullptr, libraryScope, nullptr)
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
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TableCellSizerTestFixture()  // tearDown()
    {
    }

    bool is_equal_pos(float x, float y)
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
        m_cellLayouters.push_back( nullptr );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 3,1) );

        m_cellLayouters.push_back( nullptr );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 2,1) );
        m_cellLayouters.push_back( nullptr );

        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( new MockCellLayouter(m_libraryScope, 1,1) );
        m_cellLayouters.push_back( nullptr );
        m_cellLayouters.push_back( nullptr );

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

        CHECK( is_equal_pos( sizer.my_get_height(0, 0), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(0, 1), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 0), 9.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 1), 9.0f) );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, compute_heights_2)
    {
        create_table_2();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 0, 3 /*rows*/, 5 /*cols*/);
        sizer.my_create_rowspan_table();
        sizer.my_compute_heights();

        CHECK( is_equal_pos( sizer.my_get_height(0, 0), 10.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(0, 1), 10.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(0, 2), 10.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(0, 3), 10.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(0, 4), 10.0f) );

        CHECK( is_equal_pos( sizer.my_get_height(1, 0), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 1), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 2), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 3), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 4), 12.0f) );

        CHECK( is_equal_pos( sizer.my_get_height(2, 0), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(2, 1), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(2, 2), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(2, 3), 12.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(2, 4), 12.0f) );

        delete_test_data();
    }

    TEST_FIXTURE(TableCellSizerTestFixture, logical_row_1)
    {
        create_table_1();
        MyTableCellSizer sizer(m_cellLayouters, m_heights, 1, 1 /*rows*/, 2 /*cols*/);
        sizer.my_create_rowspan_table();
        sizer.my_compute_heights();

        CHECK( is_equal_pos( sizer.my_get_height(1, 0), 9.0f) );
        CHECK( is_equal_pos( sizer.my_get_height(1, 1), 9.0f) );

        CHECK( sizer.my_get_rowspan(0, 0) == 0 );
        CHECK( sizer.my_get_rowspan(0, 1) == 0 );

        delete_test_data();
    }

};
