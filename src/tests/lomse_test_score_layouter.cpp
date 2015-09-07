//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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
#include "lomse_document_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_system_layouter.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_box_system.h"
#include "lomse_box_slice.h"
#include "lomse_calligrapher.h"
#include "lomse_instrument_engraver.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document_cursor.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


#include "lomse_time.h"
#include <cmath>


//---------------------------------------------------------------------------------------
// helper, for accesing protected members
class MyScoreLayouter : public ScoreLayouter
{
public:
    MyScoreLayouter(ImoContentObj* pImo, GraphicModel* pGModel,
                    LibraryScope& libraryScope)
        : ScoreLayouter(pImo, NULL, pGModel, libraryScope)
    {
    }
    virtual ~MyScoreLayouter() {}

    void my_create_instrument_engravers() { create_instrument_engravers(); }
    std::vector<InstrumentEngraver*>& my_get_instrument_engravers() { return m_instrEngravers; }
    void my_page_initializations(GmoBox* pBox) { page_initializations(pBox); }
    void my_move_cursor_to_top_left_corner() { move_cursor_to_top_left_corner(); }
    int my_get_num_page() { return m_iCurPage; }
//    void my_is_first_system_in_page(bool value) { is_first_system_in_page(value); }
    bool my_is_first_system_in_page() { return is_first_system_in_page(); }
    GmoBoxScorePage* my_get_current_box_page() { return m_pCurBoxPage; }
    bool my_is_first_page() { return is_first_page(); }
    std::vector<int>& my_get_line_breaks() { return m_breaks; }
    void my_decide_line_breaks() { decide_line_breaks(); }
    void my_create_system_layouter() { create_system_layouter(); }
    void my_create_system_box() { create_system_box(); }
//    SystemLayouter* my_get_current_system_layouter() { return m_pCurSysLyt; }

    LUnits my_get_target_size_for_system(int iSystem) {
        return get_target_size_for_system(iSystem);
    }
    UPoint my_get_page_cursor() { return m_cursor; }
    LUnits my_get_first_system_staves_size() { return get_first_system_staves_size(); }
    LUnits my_get_other_systems_staves_size() { return get_other_systems_staves_size();}
    LUnits my_remaining_height() { return remaining_height(); }
    void my_set_cur_system_number(int value) { m_iCurSystem = value; }
    LUnits my_distance_to_top_of_system(int iSystem, bool fFirstInPage) {
        return distance_to_top_of_system(iSystem, fFirstInPage);
    }
    bool my_enough_space_in_page() { return enough_space_in_page(); }
    ScoreMeter* my_get_score_meter() { return m_pScoreMeter; }
    GmoBoxSystem* my_get_current_system_box() { return m_pCurBoxSystem; }
    ColumnsBuilder* my_get_columns_builder() { return m_pColsBuilder; }
    ShapesCreator* my_shapes_creator() { return m_pShapesCreator; }
    ColumnLayouter* my_get_column_layouter(int iCol) { return m_ColLayouters[iCol]; }
    void my_engrave_system() { engrave_system(); }

//    void my_fill_current_system_with_columns() { fill_current_system_with_columns(); }
//    void my_justify_current_system() { justify_current_system(); }
//    SystemLayouter* my_create_system_layouter() { return create_system_layouter(); }
//    LUnits my_get_staves_height() { return m_stavesHeight; }
//    void my_determine_staves_vertical_position() { determine_staves_vertical_position(); }
//    void my_create_columns() { create_columns(); }
//    int get_num_lines_in_column(int iCol);
//    {
//        return m_ColLayouters[iCol]->get_num_lines();
//    }
//    inline std::vector<int>& get_line_breaks() { return m_breaks; }
    void my_delete_all() { delete_not_used_objects(); }
};



//---------------------------------------------------------------------------------------
class ScoreLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    FontStorage* m_pFonts;
    std::string m_scores_path;
    std::string m_sTestName;
    std::string m_sTestNum;
    Document* m_pDoc;
    DocLayouter* m_pDocLayouter;
    GraphicModel* m_pGModel;
    ScoreLayouter* m_pScoreLayouter;

    ScoreLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_pFonts( m_libraryScope.font_storage() )
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
        , m_pDocLayouter(NULL)
        , m_pGModel(NULL)
        , m_pScoreLayouter(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ScoreLayouterTestFixture()  // tearDown()
    {
        delete_test_data();
    }

    bool my_is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    ////void SaveScoreBitmap(lmGraphicManager* pGM)
    ////{
    ////    wxBitmap* pBitmap = pGM->RenderScore(1);
    ////    wxString sPath = g_pPaths->GetTestScoresPath();
    ////    wxFileName oFilename(sPath, "zimg-")+m_sTestName, "bmp"), wxPATH_NATIVE);
    ////    pBitmap->SaveFile(oFilename.GetFullPath(), wxBITMAP_TYPE_BMP);
    ////}

    void load_score_for_test(const std::string& sTestNum, const std::string& score)
    {
        delete_test_data();
        m_sTestNum = sTestNum;
        m_sTestName = score;
        std::string filename = m_scores_path + m_sTestNum + "-" + m_sTestName + ".lms";
        ifstream score_file(filename.c_str());
        CHECK( score_file.good() ) ;
        m_pDoc = LOMSE_NEW Document(m_libraryScope, cout);
        m_pDoc->from_file(filename);

        m_pDocLayouter = LOMSE_NEW DocLayouter( m_pDoc->get_im_model(), m_libraryScope);
        m_pDocLayouter->layout_document();
        m_pGModel = m_pDocLayouter->get_graphic_model();
        CHECK( m_pGModel != NULL );

        m_pScoreLayouter = m_pDocLayouter->get_score_layouter();
        CHECK( m_pScoreLayouter != NULL );
    }

    void check_line_data_equal(int iSys, int iCol, int numCols, int nSrcLine)
    {
        CHECK( m_pScoreLayouter->get_num_columns() > iCol );

        //get actual data
        std::ostringstream oss;
        for (int i=0; i < numCols; ++i)
        {
            m_pScoreLayouter->dump_column_data(iCol+i, oss);
        }
        std::string sActualData = oss.str();

        //read reference file to get expected data
        std::ostringstream ossFilename;
        ossFilename << m_scores_path << "ref-" << m_sTestNum << "-" << iSys << "-"
                    << iCol << "-" << m_sTestName << ".txt";
        string filename = ossFilename.str();
        std::ifstream ifs(filename.c_str(), std::ios::binary);
        std::string sExpectedData((std::istreambuf_iterator<char>(ifs)),
                                  std::istreambuf_iterator<char>() );

        //compare data
        if (sExpectedData != sActualData)
        {
            save_as_actual_line_data(sActualData, iSys, iCol);
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
                    "No match with expected line data");
        }
    }

    #define LOMSE_ASSERT_LINE_DATA_EQUAL( iSys, iCol, numCols )  \
                check_line_data_equal(iSys, iCol, numCols, __LINE__)


    void check_page_data_equal(int iPage, int nSrcLine)
    {
        //get actual data
        std::ostringstream oss;
        m_pGModel->dump_page(0, oss);
        std::string sActualData = oss.str();

        //read reference file to get expected data
        std::ostringstream ossFilename;
        ossFilename << m_scores_path << "ref-" << m_sTestNum << "-" << iPage
                    << "-" << m_sTestName << ".txt";
        string filename = ossFilename.str();
        std::ifstream ifs(filename.c_str(), std::ios::binary);
        std::string sExpectedData((std::istreambuf_iterator<char>(ifs)),
                                  std::istreambuf_iterator<char>() );

        //compare data
        if (sExpectedData != sActualData)
        {
            save_as_actual_page_data(sActualData, iPage);
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
                    "No match with expected page data");
        }
    }

    #define LOMSE_ASSERT_PAGE_DATA_EQUAL( iPage )  \
                check_page_data_equal(iPage, __LINE__)


    void save_as_actual_page_data(const string& sActualData, int iPage)
    {
        std::ostringstream ossFilename;
        ossFilename << m_scores_path << "dat-" << m_sTestNum << "-" << iPage
                    << "-" << m_sTestName << ".txt";
        string filename = ossFilename.str();
        std::ofstream ofs;
        ofs.open( filename.c_str(), std::ios::binary );
        ofs << sActualData;
        ofs.close();
    }

    void save_as_actual_line_data(const string& sActualData, int iSys, int iCol)
    {
        std::ostringstream ossFilename;
        ossFilename << m_scores_path << "dat-" << m_sTestNum << "-" << iSys
                    << "-" << iCol << "-" << m_sTestName << ".txt";
        string filename = ossFilename.str();
        std::ofstream ofs;
        ofs.open( filename.c_str(), std::ios::binary );
        ofs << sActualData;
        ofs.close();
    }

    void delete_test_data()
    {
        delete m_pDoc;
        m_pDoc = NULL;
        delete m_pGModel;
        m_pGModel = NULL;
        delete m_pDocLayouter;
        m_pDocLayouter = NULL;
    }
};


//---------------------------------------------------------------------------------------
// ScoreLayouter tests
//---------------------------------------------------------------------------------------

SUITE(ScoreLayouterTest)
{

    //===================================================================================
    // ScoreLayouter initialization tests
    //===================================================================================

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_Creation)
    {
        Document doc(m_libraryScope);
        //unsigned *pStack;
        //_asm {mov pStack, esp}
        //cout << "The stack is at " << pStack << endl;
        //cout << "doc is at " << &doc << endl;
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        //cout << "test precision 5798.12500: "
        //          << fixed << setprecision(2) << setfill(' ')
        //          << setw(10) << 5798.12500 << endl;

        //cout << "test precision 5798.87500: "
        //          << fixed << setprecision(2) << setfill(' ')
        //          << setw(10) << 5798.87500 << endl;

        //char buffer[250];
        //sprintf(buffer, "%.2f", 5798.12500);
        //cout << "test precison printf 5798.12500: " << buffer << endl;
        //sprintf(buffer, "%.2f", 5798.87500);
        //cout << "test precison printf 5798.87500: " << buffer << endl;

        //float num = 5798.12500f;
        //float p = floor(num * 100.0f + 0.5f) / 100.0f;
        //cout << "test precision floor 5798.12500: "
        //          << fixed << setprecision(2) << setfill(' ')
        //          << setw(10) << p << endl;
        //num = 5798.87500f;
        //p = floor(num * 100.0f + 0.5f) / 100.0f;
        //cout << "test precision floor 5798.87500: "
        //          << fixed << setprecision(2) << setfill(' ')
        //          << setw(10) << p << endl;



        CHECK( scoreLyt.my_get_columns_builder() != NULL);
        CHECK( scoreLyt.my_shapes_creator() != NULL );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateInstrEngravers)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.my_create_instrument_engravers();
        std::vector<InstrumentEngraver*>& instrEngravers =
            scoreLyt.my_get_instrument_engravers();
        CHECK( instrEngravers.size() == 1 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EmptyScoreNoColumnsOneSystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();

        CHECK( scoreLyt.get_num_columns() == 0 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateColumnsOneColumn)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();

        CHECK( scoreLyt.get_num_columns() == 1 );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateColumnsThree)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();

//        cout << "num. columns = " << scoreLyt.get_num_columns() << endl;
        CHECK( scoreLyt.get_num_columns() == 3 );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_PageInitialization)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoBoxScorePage pageBox(pImoScore);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);

        CHECK( scoreLyt.my_get_current_box_page() != NULL );
        CHECK( scoreLyt.my_get_num_page() == 0 );
        CHECK( scoreLyt.my_is_first_system_in_page() == true );
        CHECK( scoreLyt.my_is_first_page() == true );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_MoveToTopLeftCOrner)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoBoxScorePage pageBox(pImoScore);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);

        scoreLyt.my_move_cursor_to_top_left_corner();

        UPoint pageCursor = scoreLyt.my_get_page_cursor();
//        cout << "page cursor = " << pageCursor.x << ", " << pageCursor.y << endl;
        CHECK( pageCursor.x == 1500.0f );
        CHECK( pageCursor.y == 2000.0f );

        scoreLyt.my_delete_all();
    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EmptySystemHeight)
//    {
//        //Bug fix. Empty single staff has wrong height
//
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData "
//            ")) )))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//
//        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);
//
////        scoreLyt.dump_column_data(0);
////        cout << "Col width = " << pColLyt->get_gross_width() << endl;
//
//        CHECK( my_is_equal(pColLyt->get_gross_width(), 2102.14f) );
//
//        scoreLyt.my_delete_all();
//    }


    //===================================================================================
    // ColumnLayouter / LineSpacer tests
    //===================================================================================

//    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_AlignNonTimedAtProlog)
//    {
//        // should key signatures be aligned when different clefs? Currently not.
//
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(opt Render.SpacingFactor 0.4)"
//            "(instrument (staves 2)(musicData "
//            "(clef G p1)(clef F4 p2)(key E)(time 4 4)"
//            ")) )))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//
//        scoreLyt.trace_column(0, k_trace_table);
//        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);
//
//        scoreLyt.dump_column_data(0);
        ////cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        ////cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;
        ////cout << "Gross width = " << pColLyt->get_gross_width() << endl;
//
//        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
//        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 583.14f) );
//        CHECK( my_is_equal(pColLyt->get_gross_width(), 1895.14f) );
//
//        scoreLyt.my_delete_all();
//    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_AlignTimedAtStart)
//    {
//        // triplets introduce truncation errors in time comparisons. If times not
//        // properly compared, some notes will not get aligned, thus increasing
//        // column width.
//
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(opt Render.SpacingFactor 0.4)"
//            "(instrument (staves 2)(musicData "
//            "(clef G p1)(clef F4 p2)(key E)(time 4 4)"
//            "(n a3 w p1)(goBack start)"
//            "(n f2 w p2)(barline)"
//            "(n a3 q p1)"
//            "(n a3 e g+ t3)(n c4 e)(n e4 e g- t-)"
//            "(n a3 h)(goBack start)"
//            "(n a2 h p2)"
//            "(n f2 h p2)(barline)"
//            ")) )))" );
//        GraphicModel gmodel;
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        //pScore->get_staffobjs_table()->dump();
//        MyScoreLayouter scoreLyt(pScore, &gmodel, m_libraryScope);
//
//        int iCol = 2;
//        //scoreLyt.trace_column(iCol, k_trace_table);
//        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(iCol);
//
////        scoreLyt.dump_column_data(iCol);
////        cout << "Col width = " << pColLyt->get_gross_width() << endl;
//
//        CHECK( my_is_equal(pColLyt->get_gross_width(), 870.14f) );
//
//        scoreLyt.my_delete_all();
//    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_OneLine_NoVarSpaceFirstColumn)
    {
        // first column has fixed initial space and ends with a quarter note. Therefore,
        // it has variable space at end, but no var space at start

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(n d4 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        int iCol = 0;
        //scoreLyt.trace_column(iCol, k_trace_table);
        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(iCol);

        //scoreLyt.dump_column_data(iCol);
        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;

        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 596.137f) );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_OneLine_NoVarSpaceAtStart)
    {
        // second column doesn't have fixed initial space and ends with a quarter note.
        // It has variable space at end, but no var space at start

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(n d4 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(1);

//        scoreLyt.dump_column_data(1);
        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;

        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 596.137f) );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_OneLine_StartHookWidth)
    {
        // second column doesn't have fixed initial space and ends with a quarter note
        // with an accidental, that creates variable space at start.
        // It has variable space at both, start and end

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(n -d4 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(1);

        //scoreLyt.dump_column_data(1);
        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;

        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 195.00f) );
        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 596.137f) );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_OneLine_NoVarSpaces)
    {
        // first column has fixed initial space and is just a clef.
        // Therefore, it doesn't have variable space neither at start nor at end.

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);

//        scoreLyt.dump_column_data(0);
        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;
        //cout << "Gross width = " << pColLyt->get_gross_width() << endl;

        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 0.0f) );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_NoVarSpaceFirstColumn)
    {
        //Now we combine two lines to test computation of variable space at end.and
        //of column width

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(n c4 e p1 g+)(n d4 e p1 g-)(n g4 q)"
            "(goBack start)(n c3 q p2)(n d3 q)"
            ")) )))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns

        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);

//        scoreLyt.dump_column_data(0);
        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;

        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 369.425f) );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_bug80215)
    {
        // 80215. First goFwd doesn't work properly. The two initial notes
        // get positioned at same x pos

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n c4 q)(n d4 q)"
            "(goFwd h v2) )) )))" );

        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        int iCol = 0;
//        scoreLyt.trace_column(iCol, k_trace_table || k_trace_entries || k_trace_spacing);
        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//        scoreLyt.dump_column_data(iCol, cout);
        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(iCol);
        pColLyt->do_spacing();

        DocCursor cursor(&doc);
        cursor.enter_element();     //enter score. points to clef
        cursor.move_next();         //points to c4
        ImoId id1 = (*cursor)->get_id();
        cursor.move_next();         //points to d4
        ImoId id2 = (*cursor)->get_id();
        LineEntry* pEntry1 = pColLyt->get_entry_for(id1);  //first note
        LineEntry* pEntry2 = pColLyt->get_entry_for(id2);  //second note
        CHECK( pEntry1 != NULL );
        CHECK( pEntry2 != NULL );
        CHECK( pEntry2->get_position() > pEntry1->get_position() );
//        cout << "id1=" << id1 << ", id2=" << id2
//             << ", x2=" << pEntry2->get_position()
//             << ", x1=" << pEntry1->get_position() << endl;

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_space_before_clef_change)
    {
        // minimum space before clef change

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (musicData (clef G)(n f4 q)(clef F4)"
            "(n g3 q (stem down))(barline) )) )))" );

        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);

        int iCol = 0;
//        scoreLyt.trace_column(iCol, k_trace_table || k_trace_entries || k_trace_spacing);
//        scoreLyt.trace_column(1, k_trace_table || k_trace_entries || k_trace_spacing);
        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//        scoreLyt.dump_column_data(iCol, cout);
        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(iCol);
        pColLyt->do_spacing();
//        scoreLyt.dump_column_data(1, cout);

//        DocCursor cursor(&doc);
//        cursor.enter_element();     //enter score. points to clef
//        cursor.move_next();         //points to c4
//        ImoId id1 = (*cursor)->get_id();
//        cursor.move_next();         //points to d4
//        ImoId id2 = (*cursor)->get_id();
//        LineEntry* pEntry1 = pColLyt->get_entry_for(id1);  //first note
//        LineEntry* pEntry2 = pColLyt->get_entry_for(id2);  //second note
//        CHECK( pEntry1 != NULL );
//        CHECK( pEntry2 != NULL );
//        CHECK( pEntry2->get_position() > pEntry1->get_position() );
////        cout << "id1=" << id1 << ", id2=" << id2
////             << ", x2=" << pEntry2->get_position()
////             << ", x1=" << pEntry1->get_position() << endl;

        scoreLyt.my_delete_all();
    }


//    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_example_for_documentation)
//    {
//        // example for documenting spacing algorithm
//
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
//            "(instrument (staves 2) (musicData "
//            "(clef G p1)(clef F4 p2)(n c4 e v1 p1)(n e4 e p1)(n g4 q p1)"
//            "(n c3 q v2 p2)(n e3 q p2)(barline) )) )))" );
//
////        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
////            "(instrument (musicData "
////            "(clef G)(n +c4 q v1)(barline end) )) )))" );
//
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//
//        int iCol = 1;
//        scoreLyt.trace_column(iCol, k_trace_table || k_trace_entries || k_trace_spacing);
//        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//        //cout << "Num.Columns=" << scoreLyt.get_num_columns() << endl;
//        //scoreLyt.dump_column_data(iCol, cout);
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(iCol);
//        pColLyt->do_spacing();
//        scoreLyt.dump_column_data(iCol, cout);
//
////        DocCursor cursor(&doc);
////        cursor.enter_element();     //enter score. points to clef
////        cursor.move_next();         //points to c4
////        ImoId id1 = (*cursor)->get_id();
////        cursor.move_next();         //points to d4
////        ImoId id2 = (*cursor)->get_id();
////        LineEntry* pEntry1 = pColLyt->get_entry_for(id1);  //first note
////        LineEntry* pEntry2 = pColLyt->get_entry_for(id2);  //second note
////        CHECK( pEntry1 != NULL );
////        CHECK( pEntry2 != NULL );
////        CHECK( pEntry2->get_position() > pEntry1->get_position() );
//////        cout << "id1=" << id1 << ", id2=" << id2
//////             << ", x2=" << pEntry2->get_position()
//////             << ", x1=" << pEntry1->get_position() << endl;
//
//        scoreLyt.my_delete_all();
//    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ColumnLayouter_BarlineSizeAccounted)
//    {
//        //Barline size is also part of column size (bug)
//
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q)(barline end)"
//            ")) )))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//
//        scoreLyt.prepare_to_start_layout();     //this creates and layouts columns
//
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);
//
////        scoreLyt.dump_column_data(0);
////        cout << "Col width = " << pColLyt->get_gross_width() << endl;
//
//        CHECK( my_is_equal(pColLyt->get_gross_width(), 2102.14f) );
//
//        scoreLyt.my_delete_all();
//    }

    //===================================================================================
    //
    //===================================================================================



//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideBreaks)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData "
//            "(clef F4)(key E)(time 2 4)(n +c3 e.)(barline)"
//            "(n e2 q)(n e3 q)(barline)"
//            "(n f2 e (beam 1 +))(n g2 e (beam 1 -))"
//                "(n f3 e (beam 3 +))(n g3 e (beam 3 -))(barline)"
//            "(n f2 e. (beam 4 +))(n g2 s (beam 4 -b))"
//                "(n f3 s (beam 5 +f))(n g3 e. (beam 5 -))(barline)"
//            "(n g2 e. (beam 2 +))(n e3 s (beam 2 -b))(n g3 q)(barline)"
//            "(n a2 e (beam 6 +))(n g2 e (beam 6 -))(n a3 q)(barline)"
//            "(n -b2 q)(n =b3 q)(barline)"
//            "(n xc3 q)(n ++c4 q)(barline)"
//            "(n d3 q)(n --d4 q)(barline)"
//            "(n e3 q)(n e4 q)(barline)"
//            "(n f3 q)(n f4 q)(barline end)"
//            "))"
//            "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData "
//            "(clef G p1)(clef F4 p2)(key F)(time 12 8)"
//            "(n c5 e. p1)(barline)"
//            "(n e4 e p1 (beam 10 +))(n g3 e p2 (beam 10 -))"
//            "(n e4 e p1 (stem up)(beam 11 +))(n e5 e p1 (stem down)(beam 11 -))(barline)"
//            "(n e4 s p1 (beam 12 ++))(n f4 s p1 (beam 12 ==))"
//                "(n g4 s p1 (beam 12 ==))(n a4 s p1 (beam 12 --))"
//            "(n c5 q p1)(barline)"
//            "))"
//            ")))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(18000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        scoreLyt.my_decide_line_breaks();
//        std::vector<int>& breaks = scoreLyt.my_get_line_breaks();
//        //cout << "num.cols = " << scoreLyt.get_num_columns() << endl;
//        //cout << "num.systems = " << breaks.size() << endl;
//        //cout << "breaks =" << breaks[0] << ", " << breaks[1] << endl;
//        CHECK( scoreLyt.get_num_columns() == 11 );
//        CHECK( breaks.size() == 2 );
//        CHECK( breaks[0] == 0 );
//        CHECK( breaks[1] == 5 );
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateSystemBox)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        scoreLyt.my_decide_line_breaks();
//        scoreLyt.my_create_system_layouter();
//        scoreLyt.my_create_system_box();
//
//        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
//        //cout << "system box: top = " << pBox->get_top() << endl;
//        //cout << "system box: left = " << pBox->get_left() << endl;
//        //cout << "system box: width = " << pBox->get_width() << endl;
//        //cout << "system box: height = " << pBox->get_height() << endl;
//        CHECK( pBox->get_top() == 2000.0f );
//        CHECK( pBox->get_left() == 1500.0f );
//        CHECK( pBox->get_width() == 19000.0f );
//        CHECK( pBox->get_height() == 2735.0f );
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesDefault)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
//            "(n e4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
////        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
////        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
//        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
//        CHECK( scoreLyt.my_get_other_systems_staves_size() == 19000.0f );
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesCustom)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(systemLayout other (systemMargins 200 100 0 0))"
//            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
//            "(n e4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
////        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
////        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
//        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
//        CHECK( scoreLyt.my_get_other_systems_staves_size() == 18700.0f );
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInScore)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(systemLayout first (systemMargins 200 100 2200 4400))"
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
////        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
//        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 3300.0f ); //4400 - 2200/2
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInPage)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(systemLayout first (systemMargins 200 100 2200 4400))"
//            "(systemLayout other (systemMargins 200 100 2200 3400))"
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
////        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
//        CHECK( scoreLyt.my_distance_to_top_of_system(4, true) == 2300.0f ); //3400 - 2200/2
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_Other)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(systemLayout first (systemMargins 200 100 2200 4400))"
//            "(systemLayout other (systemMargins 200 100 2200 3400))"
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
////        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
//        CHECK( scoreLyt.my_distance_to_top_of_system(4, false) == 0.0f );
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_RemainingHeight)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(systemLayout first (systemMargins 200 100 2200 4400))"
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        CHECK( scoreLyt.my_remaining_height() == 23700.0f ); //page height - margin
//
//        scoreLyt.my_delete_all();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_true)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        CHECK( scoreLyt.my_enough_space_in_page() == true ); //> 1000+735
//
//        scoreLyt.my_delete_all();
//   }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_false)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(3500.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        CHECK( scoreLyt.my_remaining_height() == 1500.0f ); //page height - margin = 3900-2000 = 1900
////        cout << "remaining = " << scoreLyt.my_remaining_height() << endl;
////        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
//        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 0.0f );
//        CHECK( scoreLyt.my_enough_space_in_page() == false ); //> 1000+735
//
//        scoreLyt.my_delete_all();
//   }
//
//////    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateSystem)
//////    {
//////        Document doc(m_libraryScope);
//////        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//////            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//////        GraphicModel gmodel;
//////        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//////        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//////        scoreLyt.prepare_to_start_layout();
//////        GmoBoxScorePage pageBox(pImoScore);
//////        pageBox.set_origin(1500.0f, 2000.0f);
//////        pageBox.set_width(19000.0f);
//////        pageBox.set_height(25700.0f);
//////        scoreLyt.layout_in_box(&pageBox);
//////    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, SystemLayouter_FillSystemOneColumn)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) )) )))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.prepare_to_start_layout();
//        GmoBoxScorePage pageBox(pImoScore);
//        pageBox.set_origin(1500.0f, 2000.0f);
//        pageBox.set_width(19000.0f);
//        pageBox.set_height(25700.0f);
//        scoreLyt.my_page_initializations(&pageBox);
//        scoreLyt.my_move_cursor_to_top_left_corner();
//        scoreLyt.my_decide_line_breaks();
//        scoreLyt.my_create_system_layouter();
//        scoreLyt.my_create_system_box();
//
//        scoreLyt.my_engrave_system();
//
//        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
//        //cout << "system box: top = " << pBox->get_top() << endl;
//        //cout << "system box: left = " << pBox->get_left() << endl;
//        //cout << "system box: width = " << pBox->get_width() << endl;
//        //cout << "system box: height = " << pBox->get_height() << endl;
//        CHECK( pBox->get_top() == 2000.0f );
//        CHECK( pBox->get_left() == 1500.0f );
//        CHECK( pBox->get_width() == 19000.0f );
//        CHECK( pBox->get_height() == 2735.0f );
//
//        //scoreLyt.my_delete_all();
//    }

    //TEST_FIXTURE(ScoreLayouterTestFixture, SystemLayouter_FillSystemTwoColumns)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(n c4 q)(n +d4 q) )) )))" );
    //    GraphicModel gmodel;
    //    ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
    //    MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
    //    scoreLyt.prepare_to_start_layout();
    //    GmoBoxScorePage pageBox(pImoScore);
    //    pageBox.set_origin(1500.0f, 2000.0f);
    //    pageBox.set_width(19000.0f);
    //    pageBox.set_height(25700.0f);
    //    scoreLyt.my_page_initializations(&pageBox);
    //    scoreLyt.my_move_cursor_to_top_left_corner();
    //    scoreLyt.my_decide_line_breaks();
    //    scoreLyt.my_create_system_layouter();
    //    scoreLyt.my_create_system_box();

    //    scoreLyt.my_engrave_system();

    //    ColumnLayouter* pColLyt0 = scoreLyt.my_get_column_layouter(0);
    //    ColumnLayouter* pColLyt1 = scoreLyt.my_get_column_layouter(1);

    //    scoreLyt.dump_column_data(0);
    //    //cout << "varSpStart = " << pColLyt0->get_start_hook_width() << endl;
    //    //cout << "varSpEnd = " << pColLyt0->get_end_hook_width() << endl;
    //    //cout << "Col width = " << pColLyt0->get_gross_width() << endl;
    //    CHECK( my_is_equal(pColLyt0->get_start_hook_width(), 0.0f) );
    //    CHECK( my_is_equal(pColLyt0->get_end_hook_width(), 583.137f) );
    //    CHECK( my_is_equal(pColLyt0->get_gross_width(), 1895.14f) );

    //    scoreLyt.dump_column_data(1);
    //    cout << "varSpStart = " << pColLyt1->get_start_hook_width() << endl;
    //    //cout << "varSpEnd = " << pColLyt1->get_end_hook_width() << endl;
    //    //cout << "Col width = " << pColLyt1->get_gross_width() << endl;
    //    CHECK( my_is_equal(pColLyt1->get_start_hook_width(), 218.0f) );
    //    CHECK( my_is_equal(pColLyt1->get_end_hook_width(), 583.138f) );
    //    CHECK( my_is_equal(pColLyt1->get_gross_width(), 1078.14f) );

    //    GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
    //    //cout << "system box: top = " << pBox->get_top() << endl;
    //    //cout << "system box: left = " << pBox->get_left() << endl;
    //    //cout << "system box: width = " << pBox->get_width() << endl;
    //    //cout << "system box: height = " << pBox->get_height() << endl;
    //    CHECK( my_is_equal(pBox->get_top(), 2000.0f) );
    //    CHECK( my_is_equal(pBox->get_left(), 1500.0f) );
    //    CHECK( my_is_equal(pBox->get_width(), 19000.0f) );
    //    CHECK( my_is_equal(pBox->get_height(), 2735.0f) );

    //    GmoBoxSlice* pSlice0 = pColLyt0->get_slice_box();
    //    //cout << "column 0: top = " << pSlice0->get_top() << endl;
    //    //cout << "column 0: left = " << pSlice0->get_left() << endl;
    //    cout << "column 0: width = " << pSlice0->get_width() << endl;
    //    //cout << "column 0: height = " << pSlice0->get_height() << endl;
    //    CHECK( my_is_equal(pSlice0->get_top(), 2000.0f) );
    //    CHECK( my_is_equal(pSlice0->get_left(), 1500.0f) );
    //    CHECK( my_is_equal(pSlice0->get_width(), 1292.003f) );
    //    CHECK( my_is_equal(pSlice0->get_height(), 2735.0f) );

    //    GmoBoxSlice* pSlice1 = pColLyt1->get_slice_box();
    //    cout << "column 1: top = " << pSlice1->get_top() << endl;
    //    cout << "column 1: left = " << pSlice1->get_left() << endl;
    //    cout << "column 1: width = " << pSlice1->get_width() << endl;
    //    cout << "column 1: height = " << pSlice1->get_height() << endl;
    //    //CHECK( my_is_equal(pSlice1->get_top(), 2000.0f) );
    //    //CHECK( my_is_equal(pSlice1->get_left(), 1500.0f) );
    //    //CHECK( my_is_equal(pSlice1->get_width(), 1292.003f) );
    //    //CHECK( my_is_equal(pSlice1->get_height(), 2735.0f) );

    //    //scoreLyt.my_delete_all();
    //}

////    //TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_JustifySystem)
////    //{
////    //    Document doc(m_libraryScope);
////    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
////    //        "(instrument (musicData (clef G)(n c4 q) ))) ))" );
////    //    GraphicModel gmodel;
////    //    ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////    //    MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
////    //    scoreLyt.prepare_to_start_layout();
////    //    GmoBoxScorePage pageBox(pImoScore);
////    //    pageBox.set_origin(1500.0f, 2000.0f);
////    //    pageBox.set_width(19000.0f);
////    //    pageBox.set_height(25700.0f);
////    //    scoreLyt.my_page_initializations(&pageBox);
////    //    scoreLyt.my_move_cursor_to_top_left_corner();
////    //    scoreLyt.my_decide_line_breaks();
////
////    //    SystemLayouter* pSysLyt = scoreLyt.my_create_system_layouter();
////    //    scoreLyt.my_create_system_box(pSysLyt);
////    //    GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
////    //    scoreLyt.my_fill_current_system_with_columns();
////    //    scoreLyt.my_justify_current_system();
////    //    //cout << "system box: top = " << pBox->get_top() << endl;
////    //    //cout << "system box: left = " << pBox->get_left() << endl;
////    //    //cout << "system box: width = " << pBox->get_width() << endl;
////    //    //cout << "system box: height = " << pBox->get_height() << endl;
////    //    CHECK( pBox->get_top() == 2000.0f );
////    //    CHECK( pBox->get_left() == 1500.0f );
////    //    CHECK( pBox->get_width() == 19000.0f );
////    //    CHECK( pBox->get_height() == 2735.0f );
////    //}

#if 0
    //-----------------------------------------------------------------------------------
    //test using scores in file ---------------------------------------------------------
    //-----------------------------------------------------------------------------------

    //empty scores ----------------------------------------------------------------------

    TEST_FIXTURE(ScoreLayouterTestFixture, T00010_EmptyScoreRendersOneStaff)
    {
        //an empty score with no options only renders one staff

        load_score_for_test("00010", "empty-renders-one-staff");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00011_EmptyScoreFillPage)
    {
        //an empty score with fill option renders a page full of staves (manuscript paper)

        load_score_for_test("00011", "empty-fill-page");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00012_PageFilledWithEmptySystems)
    {
        //as 00011, but with some content in first system

        load_score_for_test("00012", "page-filled-with-empty-systems");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00013_EmptyPianoFilledWithEmptySystems)
    {
        //as 00011, but for an instrument with more than one staff

        load_score_for_test("00013", "empty-piano-filled-with-empty-systems");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    //-----------------------------------------------------------------------------------
    // Tests for LineSpacer class

    TEST_FIXTURE(ScoreLayouterTestFixture, T00020_SpaceBeforeClef)
    {
        //some space before prolog and after clef. No final variable space

        load_score_for_test("00020", "space-before-clef");
        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0, 1);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00021_SpacingInProlog)
    {
        //some space before prolog, inter symbols and after time signature,
        //no final variable space

        load_score_for_test("00021", "spacing-in-prolog");
        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0, 1);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00022_SpaceAfterPrologOneNote)
    {
        //fixed space after note. variable space at end of column

        load_score_for_test("00022", "spacing-in-prolog-one-note");
        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0, 1);
        delete_test_data();
    }

    //-----------------------------------------------------------------------------------
    // Tests for ColumnLayouter class

    TEST_FIXTURE(ScoreLayouterTestFixture, T00023_SameDurationNotesEquallySpaced)
    {
        //variable space from 1st column added at start of 2nd column
        //variable space at start taken into account

        load_score_for_test("00023", "same-duration-notes-equally-spaced");
        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0, 3);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00024_NotesSpacingProportionalToNotesDuration)
    {
        load_score_for_test("00024", "notes-spacing-proportional-to-notes-duration");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00025_FixedSpacing)
    {
        load_score_for_test("00025", "notes-with-fixed-spacing");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00026_AccidentalsDoNotAlterSpacing)
    {
        //accidentals must consume previous variable space, so that noteheads are
        //at the same distance

        load_score_for_test("00026", "accidentals-do-no-alter-spacing");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00027_AccidentalsDoNotAlterFixedSpacing)
    {
        //accidentals must consume previous variable space, so that noteheads are
        //at the same distance

        load_score_for_test("00027", "accidentals-do-no-alter-fixed-spacing");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

////////    TEST_FIXTURE(ScoreLayouterTestFixture, T00028_Spacing_notes_with_figured_bass)
////////    {
////////        load_score_for_test("00028", "spacing-notes-with-figured-bass");
////////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
////////        delete_test_data();
////////    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00030_ChordNotesAreAligned)
    {
        load_score_for_test("00030", "chord-notes-are-aligned");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00031_ChordStemUpNoteReversedNoFlag)
    {
        load_score_for_test("00031", "chord-stem-up-note-reversed-no-flag");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00032_ChordStemDownNoteReversedNoFlag)
    {
        load_score_for_test("00032", "chord-stem-down-note-reversed-no-flag");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00033_ChordsWithReversedNotesDoNotOverlap)
    {
        load_score_for_test("00033", "chords-with-reversed-notes-do-not-overlap");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00034_ChordWithAccidentalsAligned)
    {
        load_score_for_test("00034", "chord-with-accidentals-aligned");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00038_ChordsWithAccidentalsAndReversedNotesAligned)
    {
        load_score_for_test("00038", "chords-with-accidentals-and-reversed-notes-aligned");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        //LOMSE_ASSERT_LINE_DATA_EQUAL(0, 1);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00040_ClefBetweenNotesProperlySpacedWhenEnoughSpace)
    {
        load_score_for_test("00040", "clef-between-notes-properly-spaced-when-enough-space");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00041_ClefBetweenNotesProperlySpacedWhenRemovingVariableSpace)
    {
        load_score_for_test("00041", "clef-between-notes-properly-spaced-when-removing-variable-space");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00042_ClefBetweenNotesAddsLittleSpacedWhenNotEnoughSpace)
    {
        load_score_for_test("00042", "clef-between-notes-adds-little-space-when-not-enough-space");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00051_ReversedNoteInChordBeforeNote)
    {
        load_score_for_test("00051", "reversed-note-in-chord-before-note");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }


        // vertical alignment

    TEST_FIXTURE(ScoreLayouterTestFixture, T00101_VerticalRightAlignmentPrologOneNote)
    {
        load_score_for_test("00101", "vertical-right-alignment-prolog-one-note");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00102_VerticalRightAlignmentSameTimePositions)
    {
        load_score_for_test("00102", "vertical-right-alignment-same-time-positions");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00103_VerticalRightAlignmentDifferentTimePositions)
    {
        load_score_for_test("00103", "vertical-right-alignment-different-time-positions");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00104_VerticalRightAlignmentWhenAccidentalRequiresMoreSpace)
    {
        load_score_for_test("00104", "vertical-right-alignment-when-accidental-requires-more-space");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00105_VerticalRightAlignmentWhenClefsBetweenNotes)
    {
        load_score_for_test("00105", "vertical-right-alignment-when-clefs-between-notes");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00106_ClefFollowsNoteWhenNoteDisplaced)
    {
        load_score_for_test("00106", "clef-follows-note-when-note-displaced");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00107_PrologProperlyAlignedInSecondSystem)
    {
        load_score_for_test("00107", "prolog-properly-aligned-in-second-system");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

        //Gourlays' algorithm spacing problems

    TEST_FIXTURE(ScoreLayouterTestFixture, T00110_triplet_against_5_tuplet_4_14)
    {
        load_score_for_test("00110", "triplet-against-5-tuplet-4.14");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00111_loose_spacing_4_16)
    {
        load_score_for_test("00111", "loose-spacing-4.16");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00112_triplet_against_s_e_dot_4_15a)
    {
        load_score_for_test("00112", "triplet-against-s-e-dot_4.15a");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

        // systems justification (lmLineResizer object)

    TEST_FIXTURE(ScoreLayouterTestFixture, T00200_BarsGoOneAfterTheOther)
    {
        load_score_for_test("00200", "bars-go-one-after-the-other");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00201_SystemsAreJustified)
    {
        load_score_for_test("00201", "systems-are-justified");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00202_LongSingleBarIsSplitted)
    {
        load_score_for_test("00202", "long-single-bar-is-splitted");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00203_repositioning_at_justification)
    {
        load_score_for_test("00203", "repositioning-at-justification");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00204_stop_at_final_barline)
    {
        load_score_for_test("00204", "stop-at-final-barline");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00205_multimetric)
    {
        load_score_for_test("00205", "multimetric");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00206_long_bar_not_splitted)
    {
        load_score_for_test("00206", "long-bar-not-splitted");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }


        //other -------------------------------------------------------------------------

//////    TEST_FIXTURE(ScoreLayouterTestFixture, T00000_ErrorAlignRests)
//////    {
//////        load_score_for_test("00000", "error-align-rests");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }

        //Regression tests ---------------------------------------------------
        // not used to drive development. Consider them as "regression tests"

    TEST_FIXTURE(ScoreLayouterTestFixture, T80010_accidental_after_barline)
    {
        load_score_for_test("80010", "accidental-after-barline");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80011_accidentals)
//////    {
//////        load_score_for_test("80011", "accidentals");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80020_chord_no_stem_no_flag)
    {
        load_score_for_test("80020", "chord-no-stem-no-flag");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80021_chord_stem_up_no_flag)
    {
        load_score_for_test("80021", "chord-stem-up-no-flag");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80022_chord_stem_down_no_flag)
    {
        load_score_for_test("80022", "chord-stem-down-no-flag");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80025_chord_stem_up_no_flag_accidental)
    {
        load_score_for_test("80025", "chord-stem-up-no-flag-accidental");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80026_chord_flags)
//////    {
//////        load_score_for_test("80026", "chord-flags");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80027_chord_spacing)
//////    {
//////        load_score_for_test("80027", "chord-spacing");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80028_chord_notes_ordering)
    {
        load_score_for_test("80028", "chord-notes-ordering");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80030_tuplet_triplets)
    {
        load_score_for_test("80030", "tuplet-triplets");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80031_tuplet_duplets)
    {
        load_score_for_test("80031", "tuplet-duplets");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80032_tuplet_tuplet)
    {
        load_score_for_test("80032", "tuplet-tuplet");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80033_tuplet_only_bracket)
    {
        load_score_for_test("80033", "tuplet-only-bracket");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80040_beams)
    {
        load_score_for_test("80040", "beams");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T80041_chords_beamed)
    {
        load_score_for_test("80041", "chords-beamed");
        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
        delete_test_data();
    }

//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80042_beams)
//////    {
//////        load_score_for_test("80042", "beams");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80043_beam_4s_q)
//////    {
//////        load_score_for_test("80043", "beam-4s-q");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80050_ties)
//////    {
//////        load_score_for_test("80050", "ties");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80051_tie_bezier)
//////    {
//////        load_score_for_test("80051", "tie-bezier");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80052_tie_bezier_break)
//////    {
//////        load_score_for_test("80052", "tie-bezier-break");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80053_tie_bezier_barline)
//////    {
//////        load_score_for_test("80053", "tie-bezier-barline");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80054_tie_after_barline)
//////    {
//////        load_score_for_test("80054", "tie-after-barline");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80060_go_back)
//////    {
//////        load_score_for_test("80060", "go-back");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80070_some_time_signatures)
//////    {
//////        load_score_for_test("80070", "some-time-signatures");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80071_12_8_time_signature)
//////    {
//////        load_score_for_test("80071", "12-8-time-signature");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80072_2_4_time_signature)
//////    {
//////        load_score_for_test("80072", "2-4-time-signature");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80080_one_instr_2_staves)
//////    {
//////        load_score_for_test("80080", "one-instr-2-staves");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80081_two_instr_3_staves)
//////    {
//////        load_score_for_test("80081", "two-instr-3-staves");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80082_choir_STB_piano)
//////    {
//////        load_score_for_test("80082", "choir-STB-piano");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80083_critical_line)
//////    {
//////        load_score_for_test("80083", "critical-line");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80090_all_rests)
//////    {
//////        load_score_for_test("80090", "all-rests");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80091_rests_in_beam)
//////    {
//////        load_score_for_test("80091", "rests-in-beam");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80092_short_rests_in_beam)
//////    {
//////        load_score_for_test("80092", "short-rests-in-beam");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80100_spacer)
//////    {
//////        load_score_for_test("80100", "spacer");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80110_graphic_line_text)
//////    {
//////        load_score_for_test("80110", "graphic-line-text");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80111_line_after_barline)
//////    {
//////        load_score_for_test("80111", "line-after-barline");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80120_fermatas)
//////    {
//////        load_score_for_test("80120", "fermatas");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80130_metronome)
//////    {
//////        load_score_for_test("80130", "metronome");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80131_metronome)
//////    {
//////        load_score_for_test("80131", "metronome");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80132_metronome)
//////    {
//////        load_score_for_test("80132", "metronome");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80140_text)
//////    {
//////        load_score_for_test("80140", "text");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80141_text_titles)
//////    {
//////        load_score_for_test("80141", "text-titles");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80150_all_clefs)
//////    {
//////        load_score_for_test("80150", "all-clefs");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80151_all_clefs)
//////    {
//////        load_score_for_test("80151", "all-clefs");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80160_textbox)
//////    {
//////        load_score_for_test("80160", "textbox");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80161_textbox_with_anchor_line)
//////    {
//////        load_score_for_test("80161", "textbox-with-anchor-line");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80162_stacked_textboxes)
//////    {
//////        load_score_for_test("80162", "stacked-textboxes");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80170_figured_bass)
//////    {
//////        load_score_for_test("80170", "figured-bass");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80171_figured_bass_several)
//////    {
//////        load_score_for_test("80171", "figured-bass-several");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80172_figured_bass_line)
//////    {
//////        load_score_for_test("80172", "figured-bass-line");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////
//////    TEST_FIXTURE(ScoreLayouterTestFixture, T80180_new_system_tag)
//////    {
//////        load_score_for_test("80180", "new-system-tag");
//////        LOMSE_ASSERT_PAGE_DATA_EQUAL(0);
//////        delete_test_data();
//////    }
//////

#endif
}



//---------------------------------------------------------------------------------------
// ColumnsBuilder tests
//---------------------------------------------------------------------------------------

////---------------------------------------------------------------------------------------
//// helper, for accesing protected members
//class MyColumnsBuilder : public ColumnsBuilder
//{
//public:
//    MyColumnsBuilder(ImoContentObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope)
//        : SColumnsBuilder(pImo, pGModel, libraryScope)
//    {
//    }
//    virtual ~MyColumnsBuilder() {}
//};


//---------------------------------------------------------------------------------------
class ColumnsBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;

    ColumnsBuilderTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ColumnsBuilderTestFixture()  // tearDown()
    {
    }

};


SUITE(ColumnsBuilderTest)
{

//    TEST_FIXTURE(ColumnsBuilderTestFixture, ColumnsBuilder_Initialize)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        ColumnsBuilder* pColsBuilder = scoreLyt.my_get_columns_builder();
//        CHECK(pColsBuilder != NULL );
////        std::vector<int>& breaks = scoreLyt.get_line_breaks();
////        CHECK( breaks.size() == 0 );
//    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightOneStaff)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.my_initialize();
//        scoreLyt.my_determine_staves_vertical_position();
//        CHECK( scoreLyt.my_get_staves_height() == 735.0f );
//    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTwoStaves)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (staves 2)(musicData (clef G)(n c4 q) ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.my_initialize();
//        scoreLyt.my_determine_staves_vertical_position();
//        CHECK( scoreLyt.my_get_staves_height() == 2470.0f );    //1000 + 735 * 2
//    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTwoInstrTwoStaves)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))"
//            "(instrument (staff 1 (staffDistance 1500))(musicData (clef G)(n c4 q) ))"
//            ") ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.my_initialize();
//        scoreLyt.my_determine_staves_vertical_position();
//        CHECK( scoreLyt.my_get_staves_height() == 2970.0f );    //1500 + 735 * 2
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTtreeInstrFourStaves)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData (clef G)(n c4 q) ))"
//            "(instrument (staves 2)(musicData (clef G)(n c4 q) ))"
//            "(instrument (musicData (clef G)(n c4 q) ))"
//            ") ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.my_initialize();
//        scoreLyt.my_determine_staves_vertical_position();
////        cout << "height = " << scoreLyt.my_get_staves_height() << endl;
//        CHECK( scoreLyt.my_get_staves_height() == 5940.0f );    //1000 * 3 + 735 * 4
//    }
//
};



//---------------------------------------------------------------------------------------
// TimeGridTable tests
//---------------------------------------------------------------------------------------
class TimeGridTableTestFixture
{
public:

////    wxSize m_ScoreSize;
////    double m_rScale;
////    Score* m_pScore;
////    ScoreLayouter* m_pScoreLayouter;
////    BoxScore* m_pBoxScore;
////    TimeGridTable* m_pLine;
////
////
////    void load_score_for_test(const wxString& sFilename)
////    {
////        delete_test_data();
////        wxString sPath = g_pPaths->GetTestScoresPath();
////        wxFileName oFilename(sPath, sFilename, "lms"), wxPATH_NATIVE);
////        lmLDPParser parser;
////        m_pScore = parser.ParseFile( oFilename.GetFullPath() );
////        CHECK( m_pScore != NULL );
////
////        lmAggDrawer* pDrawer = LOMSE_NEW lmAggDrawer(m_ScoreSize.x, m_ScoreSize.y, m_rScale);
////        lmPaper m_oPaper;
////        m_oPaper.SetDrawer(pDrawer);
////        m_pScoreLayouter = LOMSE_NEW ScoreLayouter(&m_oPaper);
////        m_pBoxScore = m_pScore->Layout(&m_oPaper, m_pScoreLayouter);
////        SystemLayouter* pSysFmt = (SystemLayouter*) m_pScoreLayouter->GetSystemScoreLayouter(0);
////        lmColumnStorage* pColStorage = pSysFmt->GetColumnData(0);
////        m_pLine = LOMSE_NEW TimeGridTable(pColStorage);
////
////        //wxLogMessage( sFilename );
////        //wxLogMessage( m_pLine->Dump() );
////    }
////
////    void delete_test_data()
////    {
////        if (m_pScore)
////        {
////            delete m_pScore;
////            m_pScore = (lmScore*)NULL;
////        }
////        if (m_pScoreLayouter)
////        {
////            delete m_pScoreLayouter;
////            m_pScoreLayouter = (ScoreLayouter*)NULL;
////        }
////        if (m_pBoxScore)
////        {
////            delete m_pBoxScore;
////            m_pBoxScore = (lmBoxScore*)NULL;
////        }
////        if (m_pLine)
////        {
////            delete m_pLine;
////            m_pLine = (TimeGridTable*)NULL;
////        }
////    }

    // setUp
    TimeGridTableTestFixture()
    {
//        m_ScoreSize = wxSize(700, 1000);
//        m_rScale = 1.0f * lmSCALE;
//        m_pScore = (lmScore*)NULL;
//        m_pScoreLayouter = (ScoreLayouter*)NULL;
//        m_pBoxScore = (lmBoxScore*)NULL;
//        m_pLine = (TimeGridTable*)NULL;
    }

    // tearDown
    ~TimeGridTableTestFixture()
    {
//        delete_test_data();
    }

};


SUITE(TimeGridTableTest)
{

//    TEST_FIXTURE(TimeGridTableTestFixture, tablegrid_0)
//    {
//        //@ initial test: just display result
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
//            "(instrument (musicData "
//            "(clef G)(n c4 e p1 g+)(n d4 e p1 g-)(n g4 q)"
//            ")) )))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        ScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//
//        scoreLyt.layout();     //this creates and layouts columns
//
//        ColumnLayouter* pColLyt = scoreLyt.my_get_column_layouter(0);
//
////        scoreLyt.dump_column_data(0);
//        //cout << "StartHook width = " << pColLyt->get_start_hook_width() << endl;
//        //cout << "End Hook width = " << pColLyt->get_end_hook_width() << endl;
//        //cout << "Gross width = " << pColLyt->get_gross_width() << endl;
//
//        CHECK( my_is_equal(pColLyt->get_start_hook_width(), 0.0f) );
//        CHECK( my_is_equal(pColLyt->get_end_hook_width(), 356.42f) );
//
//        scoreLyt.my_delete_all();
//    }


////    TEST_FIXTURE(TimeGridTableTestFixture, empty_score_builds_empty_table)
////    {
////        load_score_for_test("00010-empty-renders-one-staff");
////        CHECK( m_pLine->GetSize() == 0 );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, just_barline_creates_one_entry)
////    {
////        load_score_for_test("90003-empty-bar-with-barline");
////        CHECK( m_pLine->GetSize() == 1 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, one_note_no_barline_creates_two_entries)
////    {
////        load_score_for_test("00022-spacing-in-prolog-one-note");
////        CHECK( m_pLine->GetSize() == 2 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 64.0f );
////        CHECK( m_pLine->GetTimepos(1) == 64.0f );
////        CHECK( m_pLine->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, three_consecutive_notes_creates_four_entries)
////    {
////        load_score_for_test("00023-same-duration-notes-equally-spaced");
////        CHECK( m_pLine->GetSize() == 4 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 64.0f );
////        CHECK( m_pLine->GetTimepos(1) == 64.0f );
////        CHECK( m_pLine->GetDuration(1) == 64.0f );
////        CHECK( m_pLine->GetTimepos(2) == 128.0f );
////        CHECK( m_pLine->GetDuration(2) == 64.0f );
////        CHECK( m_pLine->GetTimepos(3) == 192.0f );
////        CHECK( m_pLine->GetDuration(3) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, one_chord_and_barline_creates_two_entries)
////    {
////        load_score_for_test("00030-chord-notes-are-aligned");
////        CHECK( m_pLine->GetSize() == 2 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 256.0f );
////        CHECK( m_pLine->GetTimepos(1) == 256.0f );
////        CHECK( m_pLine->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, when_two_notes_at_same_time_choose_the_shortest_one)
////    {
////        load_score_for_test("90001-two-notes-different-duration");
////        CHECK( m_pLine->GetSize() == 2 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 32.0f );
////        CHECK( m_pLine->GetTimepos(1) == 32.0f );
////        CHECK( m_pLine->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, interpolate_missing_time_between_two_notes)
////    {
////        load_score_for_test("90004-two-voices-missing-timepos");
////        CHECK( m_pLine->GetSize() == 4 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 32.0f );
////        CHECK( m_pLine->GetTimepos(1) == 32.0f );
////        CHECK( m_pLine->GetDuration(1) == 0.0f );
////        CHECK( m_pLine->GetTimepos(2) == 64.0f );
////        CHECK( m_pLine->GetDuration(2) == 64.0f );
////        CHECK( m_pLine->GetTimepos(3) == 128.0f );
////        CHECK( m_pLine->GetDuration(3) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(TimeGridTableTestFixture, several_lines_with_different_durations)
////    {
////        load_score_for_test("90002-several-lines-with-different-durations");
////        CHECK( m_pLine->GetSize() == 5 );
////        CHECK( m_pLine->GetTimepos(0) == 0.0f );
////        CHECK( m_pLine->GetDuration(0) == 64.0f );
////        CHECK( m_pLine->GetTimepos(1) == 64.0f );
////        CHECK( m_pLine->GetDuration(1) == 32.0f );
////        CHECK( m_pLine->GetTimepos(2) == 96.0f );
////        CHECK( m_pLine->GetDuration(2) == 16.0f );
////        CHECK( m_pLine->GetTimepos(3) == 112.0f );
////        CHECK( m_pLine->GetDuration(3) == 16.0f );
////        CHECK( m_pLine->GetTimepos(4) == 128.0f );
////        CHECK( m_pLine->GetDuration(4) == 0.0f );
////        delete_test_data();
////    }

};
