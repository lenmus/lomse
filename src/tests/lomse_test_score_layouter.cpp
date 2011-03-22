//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_document_layouter.h"
#include "lomse_score_layouter.h"
#include "lomse_system_layouter.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_box_system.h"
#include "lomse_calligrapher.h"
#include "lomse_instrument_engraver.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper, for accesing protected members
class MyScoreLayouter : public ScoreLayouter
{
public:
    MyScoreLayouter(ImoDocObj* pImo, GraphicModel* pGModel, LibraryScope& libraryScope)
        : ScoreLayouter(pImo, pGModel, libraryScope)
    {
    }
    virtual ~MyScoreLayouter() {}

    LUnits my_get_available_space_in_system(int iSystem) {
        return get_available_space_in_system(iSystem);
    }
    void my_initialize() { initialize(); }
    void my_create_columns() { create_columns(); }
    void my_page_initializations(GmoBox* pBox) { page_initializations(pBox); }
    void my_move_cursor_to_top_left_corner() { move_cursor_to_top_left_corner(); }
    UPoint my_get_page_cursor() { return m_pageCursor; }
    LUnits my_get_first_system_staves_size() { return get_first_system_staves_size(); }
    LUnits my_get_other_systems_staves_size() { return get_other_systems_staves_size();}
    LUnits my_get_staves_height() { return m_stavesHeight; }
    void my_determine_staves_vertical_position() { determine_staves_vertical_position(); }
    void my_decide_line_breaks() { decide_line_breaks(); }
    LUnits my_remaining_height() { return remaining_height(); }
    void my_is_first_system_in_page(bool value) { is_first_system_in_page(value); }
    void my_set_cur_system_number(int value) { m_iCurSystem = value; }
    LUnits my_distance_to_top_of_system(int iSystem, bool fFirstInPage) {
        return distance_to_top_of_system(iSystem, fFirstInPage);
    }
    bool my_enough_space_in_page() { return enough_space_in_page(); }
    ScoreMeter* my_get_score_meter() { return m_pScoreMeter; }
    void my_create_system_box(SystemLayouter* pSysLyt) { create_system_box(pSysLyt); }
    GmoBoxSystem* my_get_current_system_box() { return m_pCurBoxSystem; }
    void my_fill_current_system_with_columns() { fill_current_system_with_columns(); }
    void my_justify_current_system() { justify_current_system(); }
    SystemLayouter* my_create_system_layouter() { return create_system_layouter(); }

};

//---------------------------------------------------------------------------------------
// User stories and tests for score layoutting
//
// Empty scores
// -------------
//
// 0. Depending on options, an empty score will be rendered either as a single system
//  or as page filled with empty systmed (manuscript paper).
//      + 10 EmptyScoreRendersOneStaff
//      ? 11 EmptyScoreFillPage
//
//
// Spacing a single line (LineSpacer object)
// -----------------------------------------
//
// 1. At start of score there is some spacing before the clef. The key signature and
//  the time signature follows, also with some space between them. Finally, before the
//  first note, there is some greater space.
//      + 20 space-before-clef
//      + 21 spacing-in-prolog
//      + 22 spacing-in-prolog-one-note
//
// 2. Notes of same duration are equally spaced. If some notes have accidentals, notes
//  spacing is not altered if there is enought space between notes. For notes with
//  different duration spacing is proportional to duration
//      + 23 same-duration-notes-equally-spaced
//      + 24 notes-spacing-proportional-to-notes-duration
//      + 25 notes-with_fixed-spacing
//      + 26 accidentals-do-no-alter-spacing
//      + 27 accidentals-do-no-alter-fixed-spacing
//      - 28 spacing-notes-with-figured-bass
//
// 3. Notes in chord are vertically aligned. If one note is reversed, that note should
//  not alter chords spacing unless not enough space
//      + 30 chord-notes-are-aligned
//      + 31 chord-stem-up-note-reversed-no-flag
//      + 32 chord-stem-down-note-reversed-no-flag
//      + 33 chords-with-reversed-notes-do-not-overlap
//      + 34 chord-with-accidentals-aligned
//      + 38 chords-with-accidentals-and-reversed-notes-aligned
//
// 4. Non-timed objects behave as if they were right aligned, joined to next timed
//  object. Spacing between the timed objects that enclose these non-timed ones
//  should be maintained as if the non-timed objs didn't exist. If not enough space,
//  variable space should be removed, starting from last non-timed and continuing
//  backwards.
//      + 40 clef-between-notes-properly-spaced-when-enough-space
//      + 41 clef-between-notes-properly-spaced-when-removing-variable-space
//      + 42 clef-between-notes-adds-little-space-when-not-enough-space
//      - 43 two-clefs-between-notes-adds-more-space
//      - 44 accidental-in-next-note-shifts-back-previous-clef
//      - 45 reversed-note-in-next-chord-shifts-back-previous-clef
//      - 46 accidental-in-next-chord-shifts-back-previous-clef
//
//
// Vertical alignment when the system has more than one staff (ColumnLayouter)
// ------------------------------------------------------------------------------
//
// 10. All notes at the same timepos must be vertically aligned
//      + 101 vertical-right-alignment-prolog-one-note
//      + 102 vertical-right-alignment-same-time-positions
//      ? 103 vertical-right-alignment-different-time-positions
//  *** - 104 vertical-right-alignment-when-accidental-requires-more-space
//      + 105 vertical-right-alignment-when-clefs-between-notes
//      ? 106 clef-follows-note-when-note-displaced
//      ? 107 prolog-properly-aligned-in-second-system
//
//
// Systems justification (ColumnLayouter, LineResizer)
// --------------------------------------------------------
//
// 20. A system usually contains a few bars. If not enough space for a single bar
//  split the system at timepos a common to all staves
//      ? 200 bars-go-one-after-the-other
//      ? 201 systems-are-justified
//      ? 202 long-single-bar-is-splitted
//      - 203 repositioning-at-justification
//
//
//
// Scores for regression tests
// ----------------------------
//      - 80010-accidental-after-barline
//      - 80011-accidentals
//      - 80020-chord-no-stem-no-flag
//      - 80021-chord-stem-up-no-flag
//      - 80022-chord-stem-down-no-flag
//      - 80023-chord-stem-up-note-reversed-no-flag
//      - 80024-chord-stem-down-note-reversed-no-flag
//      - 80025-chord-stem-up-no-flag-accidental
//      - 80026-chord-flags
//      - 80027-chord-spacing
//      - 80028-chord-notes-ordering
//      - 80030-tuplet-triplets
//      - 80031-tuplet-duplets
//      - 80032-tuplet-tuplet
//      - 80040-beams
//      - 80041-chords-beamed
//      - 80042-beams
//      - 80043-beam-4s-q
//      - 80050-ties
//      - 80051-tie-bezier
//      - 80052-tie-bezier-break
//      - 80053-tie-bezier-barline
//      - 80054-tie-after-barline
//      - 80060-go-back
//      - 80070-some-time-signatures
//      - 80071-12-8-time-signature
//      - 80072-2-4-time-signature
//      - 80080-one-instr-2-staves
//      - 80081-two-instr-3-staves
//      - 80082-choir-STB-piano
//      - 80083-critical-line
//      - 80090-all-rests
//      - 80091-rests-in-beam
//      - 80092-short-rests-in-beam
//      - 80100-spacer
//      - 80110-graphic-line-text
//      - 80111-line-after-barline
//      - 80120-fermatas
//      - 80130-metronome
//      - 80131-metronome
//      - 80132-metronome
//      - 80140-text
//      - 80141-text-titles
//      - 80150-all-clefs
//      - 80151-all-clefs
//      - 80160-textbox
//      - 80161-textbox-with-anchor-line
//      - 80162-stacked-textboxes
//      - 80170-figured-bass
//      - 80171-figured-bass-several
//      - 80172-figured-bass-line
//      - 80180-new-system-tag
//
// Scores for other unit tests - 9xxxx
// --------------------------------------------------------
//
// tests for class lmTimeGridTable
//      - 90001-two-notes-different-duration
//      - 90002-several-lines-with-different-durations
//      - 90003-empty-bar-with-barline
//




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
    GmoStubScore* m_pStub;
    ScoreLayouter* m_pScoreLayouter;
    SystemLayouter* m_pSysLayouter;

    ScoreLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_pFonts( m_libraryScope.font_storage() )
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
        , m_pDoc(NULL)
        , m_pDocLayouter(NULL)
        , m_pGModel(NULL)
        , m_pStub(NULL)
        , m_pScoreLayouter(NULL)
        , m_pSysLayouter(NULL)
    {
    }

    ~ScoreLayouterTestFixture()  // tearDown()
    {
        delete_test_data();
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
        m_pDoc = new Document(m_libraryScope);
        m_pDoc->from_file(filename);

        m_pDocLayouter = new DocLayouter( m_pDoc->get_im_model(), m_libraryScope);
        m_pDocLayouter->layout_document();
        m_pGModel = m_pDocLayouter->get_gm_model();
        CHECK( m_pGModel != NULL );

        m_pStub = m_pGModel->get_score_stub(0);
        CHECK( m_pStub != NULL );
        CHECK( m_pStub->get_num_pages() > 0 );
        GmoBoxScorePage* pPage = m_pStub->get_page(0);
        CHECK( pPage != NULL );

        m_pScoreLayouter
            = dynamic_cast<ScoreLayouter*>( m_pDocLayouter->get_last_layouter() );
        CHECK( m_pScoreLayouter != NULL );
        m_pSysLayouter = m_pScoreLayouter->get_system_layouter(0);
    }

    void check_line_data_equal(int iSys, int iCol, int nSrcLine)
    {
        CHECK( m_pStub->get_num_systems() > iSys );
        CHECK( m_pScoreLayouter->get_num_columns() > iCol );
        CHECK( m_pScoreLayouter->get_num_lines_in_column(iCol) > 0 );

        //get actual data
//        SystemLayouter* pSysLayouter = m_pScoreLayouter->get_system_layouter(iSys);
        std::ostringstream oss;
        m_pScoreLayouter->dump_column_data(iCol, oss);
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
            save_as_actual_data(sActualData, iSys, iCol);
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
                    "No match with expected LineTable data");
        }
    }

    #define LOMSE_ASSERT_LINE_DATA_EQUAL( iSys, iCol)  \
                check_line_data_equal(iSys, iCol, __LINE__)

    void check_score_data_equal(int nSrcLine)
    {
    //    CHECK( m_pStub->get_num_systems() > 0 );
    //    CHECK( m_pSysLayouter->get_num_columns() > 0 );

    //    //get actual data
    //    wxString sActualData = "");

    //    for (int iSys=0; iSys < m_pStub->get_num_systems(); iSys++)
    //    {
    //        SystemLayouter* pSysLay = m_pScoreLayouter->GetSystemScoreLayouter(iSys);
    //        for (int iCol=0; iCol < pSysLay->get_num_columns(); iCol++)
    //        {
    //            sActualData +=
    //                m_pScoreLayouter->GetSystemScoreLayouter(iSys)->DumpColumnData(iCol);
    //        }
    //    }

    //    //read reference file to get expected data
    //    wxString sPath = g_pPaths->GetTestScoresPath();
    //    wxString sInFile = wxString::Format("ref-%s-%s"),
    //                                        m_sTestNum.c_str(), m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sInFile, "txt"), wxPATH_NATIVE);
    //    wxString sExpectedData;
    //    wxFFile file(oFilename.GetFullPath());
    //    if ( !file.IsOpened() || !file.ReadAll(&sExpectedData) )
    //    {
    //        SaveAsActualScoreData(sActualData);
    //        UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
    //                "Reference score data cannot be read");
    //    }
    //    else
    //    {
    //        //compare data
    //        if (sExpectedData != sActualData)
    //        {
    //            SaveAsActualScoreData(sActualData);
    //            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
    //                    "No match with expected score data");
    //        }
    //    }
    }

    #define LOMSE_ASSERT_SCORE_DATA_EQUAL()  check_score_data_equal(__LINE__);


    void save_as_actual_data(const string& sActualData, int iSys, int iCol)
    {
        //cout << "save_as_actual_data" << endl;
        std::ostringstream ossFilename;
        ossFilename << m_scores_path << "dat-" << m_sTestNum << "-" << iSys << "-"
                    << iCol << "-" << m_sTestName << ".txt";
        string filename = ossFilename.str();
        std::ofstream ofs;
        ofs.open( filename.c_str(), std::ios::binary );
        ofs << sActualData;
        ofs.close();
    }

    //void SaveAsActualScoreData(const wxString& sActualData)
    //{
    //    wxString sPath = g_pPaths->GetTestScoresPath();
    //    wxString sOutFile = wxString::Format("dat-%s-%s"),
    //                                         m_sTestNum.c_str(),
    //                                         m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sOutFile, "txt"), wxPATH_NATIVE);
    //    wxFile oFile;
    //    oFile.Create(oFilename.GetFullPath(), true);    //true=overwrite
    //    oFile.Open(oFilename.GetFullPath(), wxFile::write);
    //    if (!oFile.IsOpened())
    //    {
    //        wxLogMessage("[ScoreLayouterTest::SaveAsActualScoreData] File '%s' could not be openned. Write to file cancelled"),
    //            oFilename.GetFullPath().c_str());
    //    }
    //    else
    //    {
    //        oFile.Write(sActualData);
    //        oFile.Close();
    //    }
    //}

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
//  Tests start
//---------------------------------------------------------------------------------------

SUITE(ScoreLayouterTest)
{

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_Initialize)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        CHECK( scoreLyt.get_num_columns() == 0 );
        std::vector<int>& breaks = scoreLyt.get_line_breaks();
        CHECK( breaks.size() == 0 );
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        CHECK( pStub != NULL );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightOneStaff)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_determine_staves_vertical_position();
        CHECK( scoreLyt.my_get_staves_height() == 735.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTwoStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2)(musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_determine_staves_vertical_position();
        CHECK( scoreLyt.my_get_staves_height() == 2470.0f );    //1000 + 735 * 2
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTwoInstrTwoStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))"
            "(instrument (staff 1 (staffDistance 1500))(musicData (clef G)(n c4 q) ))"
            ") ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_determine_staves_vertical_position();
        CHECK( scoreLyt.my_get_staves_height() == 2970.0f );    //1500 + 735 * 2
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_StavesHeightTtreeInstrFourStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))"
            "(instrument (staves 2)(musicData (clef G)(n c4 q) ))"
            "(instrument (musicData (clef G)(n c4 q) ))"
            ") ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_determine_staves_vertical_position();
//        cout << "height = " << scoreLyt.my_get_staves_height() << endl;
        CHECK( scoreLyt.my_get_staves_height() == 5940.0f );    //1000 * 3 + 735 * 4
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EmptyScoreNoColumnsOneSystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_create_columns();
        CHECK( scoreLyt.get_num_columns() == 0 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateColumnsOneColumn)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_create_columns();
        CHECK( scoreLyt.get_num_columns() == 1 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateColumnsThree)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.my_initialize();
        scoreLyt.my_create_columns();
        CHECK( scoreLyt.get_num_columns() == 3 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_TopLeftCOrner)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        UPoint pageCursor = scoreLyt.my_get_page_cursor();
//        cout << "page cursor = " << pageCursor.x << ", " << pageCursor.y << endl;
        CHECK( pageCursor.x == 1500.0f );
        CHECK( pageCursor.y == 2000.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesDefault)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
//        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
//        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
        CHECK( scoreLyt.my_get_other_systems_staves_size() == 19000.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DecideLineSizesCustom)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout other (systemMargins 200 100 0 0))"
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
//        cout << "first line size = " << scoreLyt.my_get_first_system_staves_size() << endl;
//        cout << "other lines size = " << scoreLyt.my_get_other_systems_staves_size() << endl;
        CHECK( scoreLyt.my_get_first_system_staves_size() == 19000.0f );
        CHECK( scoreLyt.my_get_other_systems_staves_size() == 18700.0f );
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
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        scoreLyt.my_decide_line_breaks();
        std::vector<int>& breaks = scoreLyt.get_line_breaks();
//        cout << "num.cols = " << scoreLyt.get_num_columns() << endl;
//        cout << "num.systems = " << breaks.size() << endl;
//        cout << "breaks =" << breaks[0] << ", " << breaks[1] << endl;
        CHECK( scoreLyt.get_num_columns() == 11 );
        CHECK( breaks.size() == 2 );
        CHECK( breaks[0] == 0 );
        CHECK( breaks[1] == 6 );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInScore)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 3300.0f ); //4400 - 2200/2
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_1stInPage)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(systemLayout other (systemMargins 200 100 2200 3400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(4, true) == 2300.0f ); //3400 - 2200/2
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_DistanceToTopOfSystem_Other)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(systemLayout other (systemMargins 200 100 2200 3400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(4, false) == 0.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_RemainingHeight)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(systemLayout first (systemMargins 200 100 2200 4400))"
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        CHECK( scoreLyt.my_remaining_height() == 23700.0f ); //page height - margin
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_true)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        CHECK( scoreLyt.my_enough_space_in_page() == true ); //> 1000+735
   }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_EnoughtSpaceInPage_1st_false)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(3500.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        CHECK( scoreLyt.my_remaining_height() == 1500.0f ); //page height - margin = 3900-2000 = 1900
//        cout << "remaining = " << scoreLyt.my_remaining_height() << endl;
//        cout << "distance = " <<  scoreLyt.my_distance_to_top_of_system(0, true) << endl;
        CHECK( scoreLyt.my_distance_to_top_of_system(0, true) == 0.0f );
        CHECK( scoreLyt.my_enough_space_in_page() == false ); //> 1000+735
   }

    //TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateSystem)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(n c4 q) ))) ))" );
    //    GraphicModel gmodel;
    //    ImoScore* pImoScore = doc.get_score();
    //    MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
    //    scoreLyt.prepare_to_start_layout();
    //    GmoStubScore* pStub = gmodel.get_score_stub(0);
    //    GmoBoxScorePage pageBox(pStub);
    //    pageBox.set_origin(1500.0f, 2000.0f);
    //    pageBox.set_width(19000.0f);
    //    pageBox.set_height(25700.0f);
    //    scoreLyt.layout_in_page(&pageBox);
    //}

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreateSystemBox)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        scoreLyt.my_decide_line_breaks();

        SystemLayouter* pSysLyt = scoreLyt.my_create_system_layouter();
        scoreLyt.my_create_system_box(pSysLyt);
        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
        //cout << "system box: top = " << pBox->get_top() << endl;
        //cout << "system box: left = " << pBox->get_left() << endl;
        //cout << "system box: width = " << pBox->get_width() << endl;
        //cout << "system box: height = " << pBox->get_height() << endl;
        CHECK( pBox->get_top() == 2000.0f );
        CHECK( pBox->get_left() == 1500.0f );
        CHECK( pBox->get_width() == 19000.0f );
        CHECK( pBox->get_height() == 2735.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_FillSystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        scoreLyt.my_decide_line_breaks();

        SystemLayouter* pSysLyt = scoreLyt.my_create_system_layouter();
        scoreLyt.my_create_system_box(pSysLyt);
        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
        scoreLyt.my_fill_current_system_with_columns();
        //cout << "system box: top = " << pBox->get_top() << endl;
        //cout << "system box: left = " << pBox->get_left() << endl;
        //cout << "system box: width = " << pBox->get_width() << endl;
        //cout << "system box: height = " << pBox->get_height() << endl;
        CHECK( pBox->get_top() == 2000.0f );
        CHECK( pBox->get_left() == 1500.0f );
        CHECK( pBox->get_width() == 19000.0f );
        CHECK( pBox->get_height() == 2735.0f );
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_JustifySystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q) ))) ))" );
        GraphicModel gmodel;
        ImoScore* pImoScore = doc.get_score();
        MyScoreLayouter scoreLyt(pImoScore, &gmodel, m_libraryScope);
        scoreLyt.prepare_to_start_layout();
        GmoStubScore* pStub = gmodel.get_score_stub(0);
        GmoBoxScorePage pageBox(pStub);
        pageBox.set_origin(1500.0f, 2000.0f);
        pageBox.set_width(19000.0f);
        pageBox.set_height(25700.0f);
        scoreLyt.my_page_initializations(&pageBox);
        scoreLyt.my_move_cursor_to_top_left_corner();
        scoreLyt.my_decide_line_breaks();

        SystemLayouter* pSysLyt = scoreLyt.my_create_system_layouter();
        scoreLyt.my_create_system_box(pSysLyt);
        GmoBoxSystem* pBox = scoreLyt.my_get_current_system_box();
        scoreLyt.my_fill_current_system_with_columns();
        scoreLyt.my_justify_current_system();
        //cout << "system box: top = " << pBox->get_top() << endl;
        //cout << "system box: left = " << pBox->get_left() << endl;
        //cout << "system box: width = " << pBox->get_width() << endl;
        //cout << "system box: height = " << pBox->get_height() << endl;
        CHECK( pBox->get_top() == 2000.0f );
        CHECK( pBox->get_left() == 1500.0f );
        CHECK( pBox->get_width() == 19000.0f );
        CHECK( pBox->get_height() == 2735.0f );
    }


    //-----------------------------------------------------------------------------------
    //test using scores in file ---------------------------------------------------------
    //-----------------------------------------------------------------------------------

    TEST_FIXTURE(ScoreLayouterTestFixture, T00010_EmptyScoreRendersOneStaff)
    {
        //an empty score with no options only renders one staff
        load_score_for_test("00010", "empty-renders-one-staff");
        CHECK( m_pStub->get_num_pages() == 1 );
        GmoBoxScorePage* pPage = m_pStub->get_page(0);
        CHECK( pPage != NULL );
        CHECK( m_pStub->get_num_systems() == 1 );
        GmoBoxSystem* pSys = pPage->get_system(0);
        CHECK( pSys != NULL );
        CHECK( pSys->get_num_slices() == 0 );
        delete_test_data();
    }


    //empty scores ----------------------------------------------------------------------

////    TEST_FIXTURE(ScoreLayouterTestFixture, T00011_EmptyScoreFillPage)
////    {
////        //an empty score with fill option renders a page full of staves (manuscript paper)
////
////        load_score_for_test("00011", "empty-fill-page");
////        CHECK( m_pScore != NULL );
////
////        lmGraphicManager oGraphMngr;
////        lmPaper m_oPaper;
////        oGraphMngr.PrepareToRender(m_pScore, m_ScoreSize.x, m_ScoreSize.y, m_rScale, &m_oPaper,
////                                    lmHINT_FORCE_RELAYOUT);
////
////        lmBoxScore* pBoxScore = oGraphMngr.GetBoxScore();
////
////        CHECK( m_pStub->get_num_pages() == 1 );
////        CHECK( pBoxScore->GetNumSystems() > 1 );
////
////        delete_test_data();
////    }

    //spacing a single line -------------------------------------------------------------

    TEST_FIXTURE(ScoreLayouterTestFixture, T00020_SpaceBeforeClef)
    {
        load_score_for_test("00020", "space-before-clef");
        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
        delete_test_data();
    }

//    TEST_FIXTURE(ScoreLayouterTestFixture, T00021_SpacingInProlog)
//    {
//        load_score_for_test("00021", "spacing-in-prolog");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00022_SpaceAfterPrologOneNote)
//    {
//        load_score_for_test("00022", "spacing-in-prolog-one-note");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00023_SameDurationNotesEquallySpaced)
//    {
//        load_score_for_test("00023", "same-duration-notes-equally-spaced");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00024_NotesSpacingProportionalToNotesDuration)
//    {
//        load_score_for_test("00024", "notes-spacing-proportional-to-notes-duration");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00025_FixedSpacing)
//    {
//        load_score_for_test("00025", "notes-with-fixed-spacing");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00026_AccidentalsDoNotAlterSpacing)
////    {
////        load_score_for_test("00026", "accidentals-do-no-alter-spacing");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        delete_test_data();
////    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00027_AccidentalsDoNotAlterFixedSpacing)
//    {
//        load_score_for_test("00027", "accidentals-do-no-alter-fixed-spacing");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00028_Spacing_notes_with_figured_bass)
////    {
////        load_score_for_test("00028", "spacing-notes-with-figured-bass");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        delete_test_data();
////    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00030_ChordNotesAreAligned)
//    {
//        load_score_for_test("00030", "chord-notes-are-aligned");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00031_ChordStemUpNoteReversedNoFlag)
//    {
//        load_score_for_test("00031", "chord-stem-up-note-reversed-no-flag");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00032_ChordStemDownNoteReversedNoFlag)
//    {
//        load_score_for_test("00032", "chord-stem-down-note-reversed-no-flag");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00033_ChordsWithReversedNotesDoNotOverlap)
//    {
//        load_score_for_test("00033", "chords-with-reversed-notes-do-not-overlap");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00034_ChordWithAccidentalsAligned)
//    {
//        load_score_for_test("00034", "chord-with-accidentals-aligned");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00038_ChordsWithAccidentalsAndReversedNotesAligned)
//    {
//        load_score_for_test("00038", "chords-with-accidentals-and-reversed-notes-aligned");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 1);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00040_ClefBetweenNotesProperlySpacedWhenEnoughSpace)
//    {
//        load_score_for_test("00040", "clef-between-notes-properly-spaced-when-enough-space");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00041_ClefBetweenNotesProperlySpacedWhenRemovingVariableSpace)
//    {
//        load_score_for_test("00041", "clef-between-notes-properly-spaced-when-removing-variable-space");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00042_ClefBetweenNotesAddsLittleSpacedWhenNotEnoughSpace)
//    {
//        load_score_for_test("00042", "clef-between-notes-adds-little-space-when-not-enough-space");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//        // vertical alignment
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00101_VerticalRightAlignmentPrologOneNote)
//    {
//        load_score_for_test("00101", "vertical-right-alignment-prolog-one-note");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00102_VerticalRightAlignmentSameTimePositions)
//    {
//        load_score_for_test("00102", "vertical-right-alignment-same-time-positions");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00103_VerticalRightAlignmentDifferentTimePositions)
////    {
////        load_score_for_test("00103", "vertical-right-alignment-different-time-positions");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        delete_test_data();
////    }
////
////    //TEST_FIXTURE(ScoreLayouterTestFixture, T00104_VerticalRightAlignmentWhenAccidentalRequiresMoreSpace)
////    //{
////    //    load_score_for_test("00104", "vertical-right-alignment-when-accidental-requires-more-space");
////    //    LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////    //    delete_test_data();
////    //}
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00105_VerticalRightAlignmentWhenClefsBetweenNotes)
//    {
//        load_score_for_test("00105", "vertical-right-alignment-when-clefs-between-notes");
//        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00106_ClefFollowsNoteWhenNoteDisplaced)
////    {
////        load_score_for_test("00106", "clef-follows-note-when-note-displaced");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00107_PrologProperlyAlignedInSecondSystem)
////    {
////        load_score_for_test("00107", "prolog-properly-aligned-in-second-system");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(1, 0);
////        delete_test_data();
////    }
////
////        //Gourlays' algorithm spacing problems
////        //CPPUNIT_TEST( T00110_triplet_against_5_tuplet_4_14 );
////        //CPPUNIT_TEST( T00111_loose_spacing_4_16 );
////
////    //TEST_FIXTURE(ScoreLayouterTestFixture, T00110_triplet_against_5_tuplet_4_14)
////    //{
////    //    load_score_for_test("00110", "triplet-against-5-tuplet-4.14");
////    //    LOMSE_ASSERT_SCORE_DATA_EQUAL();
////    //    delete_test_data();
////    //}
////
////        // systems justification (lmLineResizer object)
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00200_BarsGoOneAfterTheOther)
////    {
////        load_score_for_test("00200", "bars-go-one-after-the-other");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 1);
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00201_SystemsAreJustified)
////    {
////        load_score_for_test("00201", "systems-are-justified");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 1);
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00202_LongSingleBarIsSplitted)
////    {
////        load_score_for_test("00202", "long-single-bar-is-splitted");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        LOMSE_ASSERT_LINE_DATA_EQUAL(1, 0);
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00203_repositioning_at_justification)
////    {
////        load_score_for_test("00203", "repositioning-at-justification");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////        //other
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T00000_ErrorAlignRests)
////    {
////        load_score_for_test("00000", "error-align-rests");
////        LOMSE_ASSERT_LINE_DATA_EQUAL(0, 0);
////        delete_test_data();
////    }
////
////        //Regression tests ---------------------------------------------------
////        // not used to drive development. Consider them 'regression tests'
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80010_accidental_after_barline)
////    {
////        load_score_for_test("80010", "accidental-after-barline");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80011_accidentals)
////    {
////        load_score_for_test("80011", "accidentals");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80020_chord_no_stem_no_flag)
////    {
////        load_score_for_test("80020", "chord-no-stem-no-flag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80021_chord_stem_up_no_flag)
////    {
////        load_score_for_test("80021", "chord-stem-up-no-flag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80022_chord_stem_down_no_flag)
////    {
////        load_score_for_test("80022", "chord-stem-down-no-flag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80023_chord_stem_up_note_reversed_no_flag)
////    {
////        load_score_for_test("80023", "chord-stem-up-note-reversed-no-flag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80024_chord_stem_down_note_reversed_no_flag)
////    {
////        load_score_for_test("80024", "chord-stem-down-note-reversed-no-flag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80025_chord_stem_up_no_flag_accidental)
////    {
////        load_score_for_test("80025", "chord-stem-up-no-flag-accidental");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80026_chord_flags)
////    {
////        load_score_for_test("80026", "chord-flags");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80027_chord_spacing)
////    {
////        load_score_for_test("80027", "chord-spacing");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80028_chord_notes_ordering)
////    {
////        load_score_for_test("80028", "chord-notes-ordering");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80030_tuplet_triplets)
////    {
////        load_score_for_test("80030", "tuplet-triplets");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80031_tuplet_duplets)
////    {
////        load_score_for_test("80031", "tuplet-duplets");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80032_tuplet_tuplet)
////    {
////        load_score_for_test("80032", "tuplet-tuplet");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80040_beams)
////    {
////        load_score_for_test("80040", "beams");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80041_chords_beamed)
////    {
////        load_score_for_test("80041", "chords-beamed");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80042_beams)
////    {
////        load_score_for_test("80042", "beams");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80043_beam_4s_q)
////    {
////        load_score_for_test("80043", "beam-4s-q");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80050_ties)
////    {
////        load_score_for_test("80050", "ties");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80051_tie_bezier)
////    {
////        load_score_for_test("80051", "tie-bezier");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80052_tie_bezier_break)
////    {
////        load_score_for_test("80052", "tie-bezier-break");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80053_tie_bezier_barline)
////    {
////        load_score_for_test("80053", "tie-bezier-barline");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80054_tie_after_barline)
////    {
////        load_score_for_test("80054", "tie-after-barline");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80060_go_back)
////    {
////        load_score_for_test("80060", "go-back");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80070_some_time_signatures)
////    {
////        load_score_for_test("80070", "some-time-signatures");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80071_12_8_time_signature)
////    {
////        load_score_for_test("80071", "12-8-time-signature");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80072_2_4_time_signature)
////    {
////        load_score_for_test("80072", "2-4-time-signature");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80080_one_instr_2_staves)
////    {
////        load_score_for_test("80080", "one-instr-2-staves");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80081_two_instr_3_staves)
////    {
////        load_score_for_test("80081", "two-instr-3-staves");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80082_choir_STB_piano)
////    {
////        load_score_for_test("80082", "choir-STB-piano");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80083_critical_line)
////    {
////        load_score_for_test("80083", "critical-line");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80090_all_rests)
////    {
////        load_score_for_test("80090", "all-rests");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80091_rests_in_beam)
////    {
////        load_score_for_test("80091", "rests-in-beam");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80092_short_rests_in_beam)
////    {
////        load_score_for_test("80092", "short-rests-in-beam");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80100_spacer)
////    {
////        load_score_for_test("80100", "spacer");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80110_graphic_line_text)
////    {
////        load_score_for_test("80110", "graphic-line-text");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80111_line_after_barline)
////    {
////        load_score_for_test("80111", "line-after-barline");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80120_fermatas)
////    {
////        load_score_for_test("80120", "fermatas");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80130_metronome)
////    {
////        load_score_for_test("80130", "metronome");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80131_metronome)
////    {
////        load_score_for_test("80131", "metronome");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80132_metronome)
////    {
////        load_score_for_test("80132", "metronome");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80140_text)
////    {
////        load_score_for_test("80140", "text");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80141_text_titles)
////    {
////        load_score_for_test("80141", "text-titles");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80150_all_clefs)
////    {
////        load_score_for_test("80150", "all-clefs");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80151_all_clefs)
////    {
////        load_score_for_test("80151", "all-clefs");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80160_textbox)
////    {
////        load_score_for_test("80160", "textbox");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80161_textbox_with_anchor_line)
////    {
////        load_score_for_test("80161", "textbox-with-anchor-line");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80162_stacked_textboxes)
////    {
////        load_score_for_test("80162", "stacked-textboxes");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80170_figured_bass)
////    {
////        load_score_for_test("80170", "figured-bass");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80171_figured_bass_several)
////    {
////        load_score_for_test("80171", "figured-bass-several");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80172_figured_bass_line)
////    {
////        load_score_for_test("80172", "figured-bass-line");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(ScoreLayouterTestFixture, T80180_new_system_tag)
////    {
////        load_score_for_test("80180", "new-system-tag");
////        LOMSE_ASSERT_SCORE_DATA_EQUAL();
////        delete_test_data();
////    }
////
////}
////
////
////
//////-------------------------------------------------------------------------------------
////// Unit tests for class lmTimeGridTable
//////
//////  lmTimeGridTable is a table with the relation timepos <-> position for all valid
//////  positions to insert a note. The recorded positions are for the center of note heads
//////  or rests. The last position is for the barline (if exists).
//////  This object is responsible for supplying all valid timepos and their positions so
//////  that other objects (in fact only lmBoxSlice) could:
//////      a) Determine the timepos to assign to a mouse click in a certain position.
//////      b) Draw a grid of valid timepos
//////
////// tests:
//////      ? empty_score_builds_empty_table
//////      ? just_barline_creates_one_entry
//////      ? one_note_no_barline_creates_two_entries
//////      ? three_consecutive_notes_creates_four_entries
//////      ? one_chord_and_barline_creates_two_entries
//////      ? when_two_notes_at_same_time_choose_the_shortest_one
//////      ? interpolate_missing_time_between_two_notes
//////      ? several_lines_with_different_durations
//////
//////
//////-------------------------------------------------------------------------------------
////
////class lmTimeGridTableTestFixture
////{
////public:
////
////    wxSize m_ScoreSize;
////    double m_rScale;
////    lmScore* m_pScore;
////    ScoreLayouter* m_pScoreLayouter;
////    lmBoxScore* m_pBoxScore;
////    lmTimeGridTable* m_pTable;
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
////        lmAggDrawer* pDrawer = new lmAggDrawer(m_ScoreSize.x, m_ScoreSize.y, m_rScale);
////        lmPaper m_oPaper;
////        m_oPaper.SetDrawer(pDrawer);
////        m_pScoreLayouter = new ScoreLayouter(&m_oPaper);
////        m_pBoxScore = m_pScore->Layout(&m_oPaper, m_pScoreLayouter);
////        SystemLayouter* pSysFmt = (SystemLayouter*) m_pScoreLayouter->GetSystemScoreLayouter(0);
////        lmColumnStorage* pColStorage = pSysFmt->GetColumnData(0);
////        m_pTable = new lmTimeGridTable(pColStorage);
////
////        //wxLogMessage( sFilename );
////        //wxLogMessage( m_pTable->Dump() );
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
////        if (m_pTable)
////        {
////            delete m_pTable;
////            m_pTable = (lmTimeGridTable*)NULL;
////        }
////    }
////
////    // setUp
////    lmTimeGridTableTestFixture()
////    {
////        m_ScoreSize = wxSize(700, 1000);
////        m_rScale = 1.0f * lmSCALE;
////        m_pScore = (lmScore*)NULL;
////        m_pScoreLayouter = (ScoreLayouter*)NULL;
////        m_pBoxScore = (lmBoxScore*)NULL;
////        m_pTable = (lmTimeGridTable*)NULL;
////    }
////
////    // tearDown
////    ~lmTimeGridTableTestFixture()
////    {
////        delete_test_data();
////    }
////
////};
////
////
////
////SUITE(lmTimeGridTableTest)
////{
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, empty_score_builds_empty_table)
////    {
////        load_score_for_test("00010-empty-renders-one-staff");
////        CHECK( m_pTable->GetSize() == 0 );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, just_barline_creates_one_entry)
////    {
////        load_score_for_test("90003-empty-bar-with-barline");
////        CHECK( m_pTable->GetSize() == 1 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, one_note_no_barline_creates_two_entries)
////    {
////        load_score_for_test("00022-spacing-in-prolog-one-note");
////        CHECK( m_pTable->GetSize() == 2 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 64.0f );
////        CHECK( m_pTable->GetTimepos(1) == 64.0f );
////        CHECK( m_pTable->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, three_consecutive_notes_creates_four_entries)
////    {
////        load_score_for_test("00023-same-duration-notes-equally-spaced");
////        CHECK( m_pTable->GetSize() == 4 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 64.0f );
////        CHECK( m_pTable->GetTimepos(1) == 64.0f );
////        CHECK( m_pTable->GetDuration(1) == 64.0f );
////        CHECK( m_pTable->GetTimepos(2) == 128.0f );
////        CHECK( m_pTable->GetDuration(2) == 64.0f );
////        CHECK( m_pTable->GetTimepos(3) == 192.0f );
////        CHECK( m_pTable->GetDuration(3) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, one_chord_and_barline_creates_two_entries)
////    {
////        load_score_for_test("00030-chord-notes-are-aligned");
////        CHECK( m_pTable->GetSize() == 2 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 256.0f );
////        CHECK( m_pTable->GetTimepos(1) == 256.0f );
////        CHECK( m_pTable->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, when_two_notes_at_same_time_choose_the_shortest_one)
////    {
////        load_score_for_test("90001-two-notes-different-duration");
////        CHECK( m_pTable->GetSize() == 2 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 32.0f );
////        CHECK( m_pTable->GetTimepos(1) == 32.0f );
////        CHECK( m_pTable->GetDuration(1) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, interpolate_missing_time_between_two_notes)
////    {
////        load_score_for_test("90004-two-voices-missing-timepos");
////        CHECK( m_pTable->GetSize() == 4 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 32.0f );
////        CHECK( m_pTable->GetTimepos(1) == 32.0f );
////        CHECK( m_pTable->GetDuration(1) == 0.0f );
////        CHECK( m_pTable->GetTimepos(2) == 64.0f );
////        CHECK( m_pTable->GetDuration(2) == 64.0f );
////        CHECK( m_pTable->GetTimepos(3) == 128.0f );
////        CHECK( m_pTable->GetDuration(3) == 0.0f );
////        delete_test_data();
////    }
////
////    TEST_FIXTURE(lmTimeGridTableTestFixture, several_lines_with_different_durations)
////    {
////        load_score_for_test("90002-several-lines-with-different-durations");
////        CHECK( m_pTable->GetSize() == 5 );
////        CHECK( m_pTable->GetTimepos(0) == 0.0f );
////        CHECK( m_pTable->GetDuration(0) == 64.0f );
////        CHECK( m_pTable->GetTimepos(1) == 64.0f );
////        CHECK( m_pTable->GetDuration(1) == 32.0f );
////        CHECK( m_pTable->GetTimepos(2) == 96.0f );
////        CHECK( m_pTable->GetDuration(2) == 16.0f );
////        CHECK( m_pTable->GetTimepos(3) == 112.0f );
////        CHECK( m_pTable->GetDuration(3) == 16.0f );
////        CHECK( m_pTable->GetTimepos(4) == 128.0f );
////        CHECK( m_pTable->GetDuration(4) == 0.0f );
////        delete_test_data();
////    }

};
