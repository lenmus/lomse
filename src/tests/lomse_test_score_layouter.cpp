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
//#include "lomse_spacing_algorithm_timetable.h"
#include "lomse_timegrid_table.h"

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

    std::vector<InstrumentEngraver*>& my_get_instrument_engravers() {
        return m_pPartsEngraver->dbg_get_instrument_engravers();
    }
    void my_initialice_score_layouter() { initialice_score_layouter(); }
    void my_page_initializations(GmoBox* pBox) { page_initializations(pBox); }
    void my_move_cursor_to_top_left_corner() { move_cursor_to_top_left_corner(); }
    int my_get_num_page() { return m_iCurPage; }
    bool my_is_first_system_in_page() { return is_first_system_in_page(); }
    GmoBoxScorePage* my_get_current_box_page() { return m_pCurBoxPage; }
    bool my_is_first_page() { return is_first_page(); }
    std::vector<int>& my_get_line_breaks() { return m_breaks; }
    void my_decide_line_breaks() { decide_line_breaks(); }
    void my_create_system_layouter() { create_system_layouter(); }
    void my_create_system_box() { create_system_box(); }

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
    bool my_enough_space_in_page() { return enough_space_for_empty_system(); }
    ScoreMeter* my_get_score_meter() { return m_pScoreMeter; }
    GmoBoxSystem* my_get_current_system_box() { return m_pCurBoxSystem; }
    ShapesCreator* my_shapes_creator() { return m_pShapesCreator; }
    void my_engrave_system() { engrave_system(); }

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

    inline bool my_is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
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

//    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_initialize)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
//            "(instrument (musicData ))) ))" );
//        GraphicModel gmodel;
//        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
//        scoreLyt.my_initialice_score_layouter();
//
//        CHECK( scoreLyt.my_get_columns_builder() != NULL);
//        CHECK( scoreLyt.my_shapes_creator() != NULL );
//        std::vector<InstrumentEngraver*>& instrEngravers =
//            scoreLyt.my_get_instrument_engravers();
//        CHECK( instrEngravers.size() == 1 );
//    }

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

//        cout << test_name() << endl;
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
//        cout << test_name() << endl;
//        cout << "page cursor = " << pageCursor.x << ", " << pageCursor.y << endl;
        CHECK( pageCursor.x == 1500.0f );
        CHECK( pageCursor.y == 2000.0f );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideBreaks)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (name \"Violin\")(abbrev \"Vln.\")(musicData "
            "(clef F4)(key E)(time 2 4)(n +c3 e.)(barline)"
            "(n e2 q)(n e3 q)(barline)"
            "(n f2 e (beam 1 +))(n g2 e (beam 1 -))"
                "(n f3 e (beam 3 +))(n g3 e (beam 3 -))(barline)"
            "(n f2 e. (beam 4 +))(n g2 s (beam 4 -b))"
                "(n f3 s (beam 5 +f))(n g3 e. (beam 5 -))(barline)"
            "(n g2 e. (beam 2 +))(n e3 s (beam 2 -b))(n g3 q)(barline)"
            "(n a2 e (beam 6 +))(n g2 e (beam 6 -))(n a3 q)(barline)"
            "(n -b2 q)(n =b3 q)(barline)"
            "(n xc3 q)(n ++c4 q)(barline)"
            "(n d3 q)(n --d4 q)(barline)"
            "(n e3 q)(n e4 q)(barline)"
            "(n f3 q)(n f4 q)(barline end)"
            "))"
            "(instrument (name \"pilano\")(abbrev \"P\")(staves 2)(musicData "
            "(clef G p1)(clef F4 p2)(key F)(time 12 8)"
            "(n c5 e. p1)(barline)"
            "(n e4 e p1 (beam 10 +))(n g3 e p2 (beam 10 -))"
            "(n e4 e p1 (stem up)(beam 11 +))(n e5 e p1 (stem down)(beam 11 -))(barline)"
            "(n e4 s p1 (beam 12 ++))(n f4 s p1 (beam 12 ==))"
                "(n g4 s p1 (beam 12 ==))(n a4 s p1 (beam 12 --))"
            "(n c5 q p1)(barline)"
            "))"
            ")))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoBoxScorePage pageBox(pImoScore);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(18000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        scoreLyt.my_decide_line_breaks();
        std::vector<int>& breaks = scoreLyt.my_get_line_breaks();
//        cout << test_name() << endl;
//        cout << "num.cols = " << scoreLyt.get_num_columns() << endl;
//        cout << "num.systems = " << breaks.size() << endl;
//        cout << "breaks =" << breaks[0] << ", " << breaks[1] << endl;
        CHECK( scoreLyt.get_num_columns() == 4 );
        CHECK( breaks.size() == 2 );
        CHECK( breaks[0] == 0 );
        CHECK( breaks[1] == 3 );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateSystemBox)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
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
        scoreLyt.my_decide_line_breaks();
        scoreLyt.my_create_system_layouter();
        scoreLyt.my_create_system_box();

        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
//        cout << test_name() << endl;
        //cout << "system box: top = " << pBox->get_top() << endl;
        //cout << "system box: left = " << pBox->get_left() << endl;
        //cout << "system box: width = " << pBox->get_width() << endl;
        //cout << "system box: height = " << pBox->get_height() << endl;
        CHECK( pBox->get_top() == 2000.0f );
        CHECK( pBox->get_left() == 1500.0f );
        CHECK( pBox->get_width() == 19000.0f );
        CHECK( pBox->get_height() == 2735.0f );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesDefault)
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
//        cout << test_name() << endl;
//        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
//        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
        CHECK( scoreLyt.my_get_other_systems_staves_size() == 19000.0f );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesCustom)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout other (systemMargins 200 100 0 0))"
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
//        cout << test_name() << endl;
//        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
//        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
        CHECK( scoreLyt.my_get_other_systems_staves_size() == 18700.0f );

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInScore)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
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
//        cout << test_name() << endl;
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 3300.0f ); //4400 - 2200/2

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInPage)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(systemLayout other (systemMargins 200 100 2200 3400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
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
//        cout << test_name() << endl;
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(4, true) == 2300.0f ); //3400 - 2200/2

        scoreLyt.my_delete_all();
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_Other)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(systemLayout other (systemMargins 200 100 2200 3400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
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
//        cout << test_name() << endl;
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(4, false) == 0.0f );

        scoreLyt.my_delete_all();
    }

//Next test is wrong. Available space is not page height - 2000. Why 2000?
//==============================================================================
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
//        cout << test_name() << endl;
//        cout << "remaining = " << scoreLyt.my_remaining_height() << endl;
//
//        scoreLyt.my_delete_all();
//    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_true)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
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
        CHECK( scoreLyt.my_enough_space_in_page() == true ); //> 1000+735

        scoreLyt.my_delete_all();
   }

//Next test is wrong. Available space is not 3900-2000. Why these numbers?
//==============================================================================
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
//        cout << test_name() << endl;
//        cout << "remaining = " << scoreLyt.my_remaining_height() << endl;
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
//        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 0.0f );
//        CHECK( scoreLyt.my_enough_space_in_page() == false ); //> 1000+735
//
//        scoreLyt.my_delete_all();
//   }

    TEST_FIXTURE(ScoreLayouterTestFixture, SystemLayouter_FillSystemOneColumn)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) )) )))" );
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
        scoreLyt.my_decide_line_breaks();
        scoreLyt.my_create_system_layouter();
        scoreLyt.my_create_system_box();

        scoreLyt.my_engrave_system();

        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
//        cout << test_name() << endl;
        //cout << "system box: top = " << pBox->get_top() << endl;
        //cout << "system box: left = " << pBox->get_left() << endl;
        //cout << "system box: width = " << pBox->get_width() << endl;
        //cout << "system box: height = " << pBox->get_height() << endl;
        CHECK( pBox->get_top() == 2000.0f );
        CHECK( pBox->get_left() == 1500.0f );
        CHECK( pBox->get_width() == 19000.0f );
        CHECK( pBox->get_height() == 2735.0f );

        //scoreLyt.my_delete_all();
    }

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

