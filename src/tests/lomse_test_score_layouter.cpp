//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_box_system.h"
#include "lomse_calligrapher.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ScoreLayouterTestFixture
{
public:
    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;
    //wxSize m_ScoreSize;
    //double m_rScale;
    //lmScore* m_pScore;
    std::string m_sTestName;
    std::string m_sTestNum;
    //ScoreLayouter* m_pScoreLayouter;
    //lmBoxScore* m_pBoxScore;
    //lmSystemScoreLayouter* m_pSysFmt;
    DocLayouter* m_pDocLayouter;


    ScoreLayouterTestFixture()   // setUp()
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
        m_scores_path = LOMSE_TEST_SCORES_PATH;

        m_pDocLayouter = NULL;
        //m_ScoreSize = wxSize(700, 1000);
        //m_rScale = 1.0f * lmSCALE;
        //m_pScore = (lmScore*)NULL;
        //m_pScoreLayouter = (ScoreLayouter*)NULL;
        //m_pBoxScore = (lmBoxScore*)NULL;
    }

    ~ScoreLayouterTestFixture()  // tearDown()
    {
        delete m_pLibraryScope;
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
        Document doc(*m_pLibraryScope);
        doc.from_file(filename);
        TextMeter meter( m_pLibraryScope->font_storage() );
        m_pDocLayouter = new DocLayouter( doc.get_im_model(), &meter);
        m_pDocLayouter->layout_document();
    }

    //void CheckLineDataEqual(int iSys, int iCol, int nSrcLine)
    //{
    //    CHECK( m_pBoxScore != NULL );
    //    CHECK( m_pBoxScore->GetNumPages() > 0 );
    //    CHECK( m_pBoxScore->GetNumSystems() > iSys );
    //    CHECK( m_pSysFmt->GetNumColumns() > iCol );
    //    CHECK( m_pSysFmt->GetNumLinesInColumn(iCol) > 0 );

    //    //get actual data
    //    wxString sActualData =
    //        m_pScoreLayouter->GetSystemScoreLayouter(iSys)->DumpColumnData(iCol);

    //    //read reference file to get expected data
    //    wxString sPath = g_pPaths->GetTestScoresPath();
    //    wxString sInFile = wxString::Format("ref-%s-%d-%d-%s"),
    //                            m_sTestNum.c_str(), iSys, iCol, m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sInFile, "txt"), wxPATH_NATIVE);
    //    wxString sExpectedData;
    //    wxFFile file(oFilename.GetFullPath());
    //    if ( !file.IsOpened() || !file.ReadAll(&sExpectedData) )
    //    {
    //        SaveAsActualData(sActualData, iSys, iCol);
    //        UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
    //                "Reference LineTable data cannot be read");
    //    }
    //    else
    //    {
    //        //compare data
    //        if (sExpectedData != sActualData)
    //        {
    //            SaveAsActualData(sActualData, iSys, iCol);
    //            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), nSrcLine),
    //                    "No match with expected LineTable data");
    //        }
    //    }
    //}

    //#define LM_ASSERT_LINE_DATA_EQUAL( iSys, iCol)  CheckLineDataEqual(iSys, iCol, __LINE__)

    //void CheckScoreDataEqual(int nSrcLine)
    //{
    //    CHECK( m_pBoxScore != NULL );
    //    CHECK( m_pBoxScore->GetNumPages() > 0 );
    //    CHECK( m_pBoxScore->GetNumSystems() > 0 );
    //    CHECK( m_pSysFmt->GetNumColumns() > 0 );

    //    //get actual data
    //    wxString sActualData = "");

    //    for (int iSys=0; iSys < m_pBoxScore->GetNumSystems(); iSys++)
    //    {
    //        lmSystemScoreLayouter* pSysFmt = m_pScoreLayouter->GetSystemScoreLayouter(iSys);
    //        for (int iCol=0; iCol < pSysFmt->GetNumColumns(); iCol++)
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
    //}

    //#define LM_ASSERT_SCORE_DATA_EQUAL()  CheckScoreDataEqual(__LINE__);


    //void SaveAsActualData(const wxString& sActualData, int iSys, int iCol)
    //{
    //    wxString sPath = g_pPaths->GetTestScoresPath();
    //    wxString sOutFile = wxString::Format("dat-%s-%d-%d-%s"),
    //                                            m_sTestNum.c_str(),
    //                                            iSys, iCol, m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sOutFile, "txt"), wxPATH_NATIVE);
    //    wxFile oFile;
    //    oFile.Create(oFilename.GetFullPath(), true);    //true=overwrite
    //    oFile.Open(oFilename.GetFullPath(), wxFile::write);
    //    if (!oFile.IsOpened())
    //    {
    //        wxLogMessage("[ScoreLayouterTest::SaveAsActualData] File '%s' could not be openned. Write to file cancelled"),
    //            oFilename.GetFullPath().c_str());
    //    }
    //    else
    //    {
    //        oFile.Write(sActualData);
    //        oFile.Close();
    //    }
    //}

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
        //if (m_pScore)
        //{
        //    delete m_pScore;
        //    m_pScore = (lmScore*)NULL;
        //}
        if (m_pDocLayouter)
        {
            delete m_pDocLayouter;
            m_pDocLayouter = NULL;
        }
        //if (m_pBoxScore)
        //{
        //    delete m_pBoxScore;
        //    m_pBoxScore = (lmBoxScore*)NULL;
        //}
    }
};


//==============================================================
//  Tests start 
//==============================================================


SUITE(ScoreLayouterTest)
{

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_CreatesScoreStub)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        TextMeter meter( m_pLibraryScope->font_storage() );
        DocLayouter dl( doc.get_im_model(), &meter);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoStubScore* pStub = pGModel->get_score_stub(0);
        CHECK( pStub != NULL );
        delete pGModel;
    }

    TEST_FIXTURE(ScoreLayouterTestFixture, T00010_EmptyScoreRendersOneStaff)
    {
        //an empty score with no options only renders one staff
        load_score_for_test("00010", "empty-renders-one-staff");
        GraphicModel* pGModel = m_pDocLayouter->get_gm_model();
        GmoStubScore* pStub = pGModel->get_score_stub(0);
        CHECK( pStub != NULL );
        CHECK( pStub->get_num_pages() == 1 );
        GmoBoxScorePage* pPage = pStub->get_page(0);
        CHECK( pPage != NULL );
        CHECK( pStub->get_num_systems() == 1 );
        GmoBoxSystem* pSys = pPage->get_system(0);
        CHECK( pSys != NULL );
        CHECK( pSys->get_num_slices() == 1 );
        delete pGModel;
        delete_test_data();
    }


//            //empty scores
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00010_EmptyScoreRendersOneStaff)
//    {
//        //an empty score with no options only renders one staff
//        load_score_for_test("00010", "empty-renders-one-staff");
//        CHECK( m_pSysFmt->GetNumObjectsInColumnLine(0, 0) == 1 );
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00011_EmptyScoreFillPage)
//    {
//        //an empty score with fill option renders a page full of staves (manuscript paper)
//
//        load_score_for_test("00011", "empty-fill-page");
//        CHECK( m_pScore != NULL );
//
//        lmGraphicManager oGraphMngr;
//        lmPaper m_oPaper;
//        oGraphMngr.PrepareToRender(m_pScore, m_ScoreSize.x, m_ScoreSize.y, m_rScale, &m_oPaper,
//                                    lmHINT_FORCE_RELAYOUT);
//
//        lmBoxScore* pBoxScore = oGraphMngr.GetBoxScore();
//
//        CHECK( pBoxScore != NULL );
//        CHECK( pBoxScore->GetNumPages() == 1 );
//        CHECK( pBoxScore->GetNumSystems() > 1 );
//
//        delete_test_data();
//    }
//
//        //spacing a single line
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00020_SpaceBeforeClef)
//    {
//        load_score_for_test("00020", "space-before-clef");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00021_SpacingInProlog)
//    {
//        load_score_for_test("00021", "spacing-in-prolog");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00022_SpaceAfterPrologOneNote)
//    {
//        load_score_for_test("00022", "spacing-in-prolog-one-note");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00023_SameDurationNotesEquallySpaced)
//    {
//        load_score_for_test("00023", "same-duration-notes-equally-spaced");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00024_NotesSpacingProportionalToNotesDuration)
//    {
//        load_score_for_test("00024", "notes-spacing-proportional-to-notes-duration");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00025_AccidentalsDoNotAlterSpacing)
//    {
//        load_score_for_test("00025", "accidentals-do-no-alter-spacing");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00026_AccidentalsDoNotAlterFixedSpacing)
//    {
//        load_score_for_test("00026", "accidentals-do-no-alter-fixed-spacing");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00027_Spacing_notes_with_figured_bass)
//    {
//        load_score_for_test("00027", "spacing-notes-with-figured-bass");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00030_ChordNotesAreAligned)
//    {
//        load_score_for_test("00030", "chord-notes-are-aligned");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00031_ChordStemUpNoteReversedNoFlag)
//    {
//        load_score_for_test("00031", "chord-stem-up-note-reversed-no-flag");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00032_ChordStemDownNoteReversedNoFlag)
//    {
//        load_score_for_test("00032", "chord-stem-down-note-reversed-no-flag");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00033_ChordsWithReversedNotesDoNotOverlap)
//    {
//        load_score_for_test("00033", "chords-with-reversed-notes-do-not-overlap");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00034_ChordWithAccidentalsAligned)
//    {
//        load_score_for_test("00034", "chord-with-accidentals-aligned");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00038_ChordsWithAccidentalsAndReversedNotesAligned)
//    {
//        load_score_for_test("00038", "chords-with-accidentals-and-reversed-notes-aligned");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00040_ClefBetweenNotesProperlySpacedWhenEnoughSpace)
//    {
//        load_score_for_test("00040", "clef-between-notes-properly-spaced-when-enough-space");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00041_ClefBetweenNotesProperlySpacedWhenRemovingVariableSpace)
//    {
//        load_score_for_test("00041", "clef-between-notes-properly-spaced-when-removing-variable-space");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00042_ClefBetweenNotesAddsLittleSpacedWhenNotEnoughSpace)
//    {
//        load_score_for_test("00042", "clef-between-notes-adds-little-space-when-not-enough-space");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//        // vertical alignment
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00101_VerticalRightAlignmentPrologOneNote)
//    {
//        load_score_for_test("00101", "vertical-right-alignment-prolog-one-note");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00102_VerticalRightAlignmentSameTimePositions)
//    {
//        load_score_for_test("00102", "vertical-right-alignment-same-time-positions");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00103_VerticalRightAlignmentDifferentTimePositions)
//    {
//        load_score_for_test("00103", "vertical-right-alignment-different-time-positions");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    //TEST_FIXTURE(ScoreLayouterTestFixture, T00104_VerticalRightAlignmentWhenAccidentalRequiresMoreSpace)
//    //{
//    //    load_score_for_test("00104", "vertical-right-alignment-when-accidental-requires-more-space");
//    //    LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//    //    delete_test_data();
//    //}
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00105_VerticalRightAlignmentWhenClefsBetweenNotes)
//    {
//        load_score_for_test("00105", "vertical-right-alignment-when-clefs-between-notes");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00106_ClefFollowsNoteWhenNoteDisplaced)
//    {
//        load_score_for_test("00106", "clef-follows-note-when-note-displaced");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00107_PrologProperlyAlignedInSecondSystem)
//    {
//        load_score_for_test("00107", "prolog-properly-aligned-in-second-system");
//        LM_ASSERT_LINE_DATA_EQUAL(1, 0);
//        delete_test_data();
//    }
//
//        //Gourlays' algorithm spacing problems
//        //CPPUNIT_TEST( T00110_triplet_against_5_tuplet_4_14 );
//        //CPPUNIT_TEST( T00111_loose_spacing_4_16 );
//
//    //TEST_FIXTURE(ScoreLayouterTestFixture, T00110_triplet_against_5_tuplet_4_14)
//    //{
//    //    load_score_for_test("00110", "triplet-against-5-tuplet-4.14");
//    //    LM_ASSERT_SCORE_DATA_EQUAL();
//    //    delete_test_data();
//    //}
//
//        // systems justification (lmLineResizer object)
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00200_BarsGoOneAfterTheOther)
//    {
//        load_score_for_test("00200", "bars-go-one-after-the-other");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00201_SystemsAreJustified)
//    {
//        load_score_for_test("00201", "systems-are-justified");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00202_LongSingleBarIsSplitted)
//    {
//        load_score_for_test("00202", "long-single-bar-is-splitted");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(1, 0);
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00203_repositioning_at_justification)
//    {
//        load_score_for_test("00203", "repositioning-at-justification");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//        //other
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00000_ErrorAlignRests)
//    {
//        load_score_for_test("00000", "error-align-rests");
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        delete_test_data();
//    }
//
//        //Regression tests ---------------------------------------------------
//        // not used to drive development. Consider them 'regression tests'
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80010_accidental_after_barline)
//    {
//        load_score_for_test("80010", "accidental-after-barline");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80011_accidentals)
//    {
//        load_score_for_test("80011", "accidentals");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80020_chord_no_stem_no_flag)
//    {
//        load_score_for_test("80020", "chord-no-stem-no-flag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80021_chord_stem_up_no_flag)
//    {
//        load_score_for_test("80021", "chord-stem-up-no-flag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80022_chord_stem_down_no_flag)
//    {
//        load_score_for_test("80022", "chord-stem-down-no-flag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80023_chord_stem_up_note_reversed_no_flag)
//    {
//        load_score_for_test("80023", "chord-stem-up-note-reversed-no-flag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80024_chord_stem_down_note_reversed_no_flag)
//    {
//        load_score_for_test("80024", "chord-stem-down-note-reversed-no-flag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80025_chord_stem_up_no_flag_accidental)
//    {
//        load_score_for_test("80025", "chord-stem-up-no-flag-accidental");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80026_chord_flags)
//    {
//        load_score_for_test("80026", "chord-flags");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80027_chord_spacing)
//    {
//        load_score_for_test("80027", "chord-spacing");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80028_chord_notes_ordering)
//    {
//        load_score_for_test("80028", "chord-notes-ordering");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80030_tuplet_triplets)
//    {
//        load_score_for_test("80030", "tuplet-triplets");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80031_tuplet_duplets)
//    {
//        load_score_for_test("80031", "tuplet-duplets");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80032_tuplet_tuplet)
//    {
//        load_score_for_test("80032", "tuplet-tuplet");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80040_beams)
//    {
//        load_score_for_test("80040", "beams");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80041_chords_beamed)
//    {
//        load_score_for_test("80041", "chords-beamed");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80042_beams)
//    {
//        load_score_for_test("80042", "beams");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80043_beam_4s_q)
//    {
//        load_score_for_test("80043", "beam-4s-q");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80050_ties)
//    {
//        load_score_for_test("80050", "ties");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80051_tie_bezier)
//    {
//        load_score_for_test("80051", "tie-bezier");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80052_tie_bezier_break)
//    {
//        load_score_for_test("80052", "tie-bezier-break");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80053_tie_bezier_barline)
//    {
//        load_score_for_test("80053", "tie-bezier-barline");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80054_tie_after_barline)
//    {
//        load_score_for_test("80054", "tie-after-barline");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80060_go_back)
//    {
//        load_score_for_test("80060", "go-back");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80070_some_time_signatures)
//    {
//        load_score_for_test("80070", "some-time-signatures");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80071_12_8_time_signature)
//    {
//        load_score_for_test("80071", "12-8-time-signature");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80072_2_4_time_signature)
//    {
//        load_score_for_test("80072", "2-4-time-signature");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80080_one_instr_2_staves)
//    {
//        load_score_for_test("80080", "one-instr-2-staves");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80081_two_instr_3_staves)
//    {
//        load_score_for_test("80081", "two-instr-3-staves");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80082_choir_STB_piano)
//    {
//        load_score_for_test("80082", "choir-STB-piano");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80083_critical_line)
//    {
//        load_score_for_test("80083", "critical-line");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80090_all_rests)
//    {
//        load_score_for_test("80090", "all-rests");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80091_rests_in_beam)
//    {
//        load_score_for_test("80091", "rests-in-beam");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80092_short_rests_in_beam)
//    {
//        load_score_for_test("80092", "short-rests-in-beam");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80100_spacer)
//    {
//        load_score_for_test("80100", "spacer");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80110_graphic_line_text)
//    {
//        load_score_for_test("80110", "graphic-line-text");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80111_line_after_barline)
//    {
//        load_score_for_test("80111", "line-after-barline");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80120_fermatas)
//    {
//        load_score_for_test("80120", "fermatas");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80130_metronome)
//    {
//        load_score_for_test("80130", "metronome");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80131_metronome)
//    {
//        load_score_for_test("80131", "metronome");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80132_metronome)
//    {
//        load_score_for_test("80132", "metronome");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80140_text)
//    {
//        load_score_for_test("80140", "text");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80141_text_titles)
//    {
//        load_score_for_test("80141", "text-titles");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80150_all_clefs)
//    {
//        load_score_for_test("80150", "all-clefs");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80151_all_clefs)
//    {
//        load_score_for_test("80151", "all-clefs");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80160_textbox)
//    {
//        load_score_for_test("80160", "textbox");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80161_textbox_with_anchor_line)
//    {
//        load_score_for_test("80161", "textbox-with-anchor-line");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80162_stacked_textboxes)
//    {
//        load_score_for_test("80162", "stacked-textboxes");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80170_figured_bass)
//    {
//        load_score_for_test("80170", "figured-bass");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80171_figured_bass_several)
//    {
//        load_score_for_test("80171", "figured-bass-several");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80172_figured_bass_line)
//    {
//        load_score_for_test("80172", "figured-bass-line");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80180_new_system_tag)
//    {
//        load_score_for_test("80180", "new-system-tag");
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        delete_test_data();
//    }
//
//}
//
//
//
////-------------------------------------------------------------------------------------
//// Unit tests for class lmTimeGridTable
////
////  lmTimeGridTable is a table with the relation timepos <-> position for all valid
////  positions to insert a note. The recorded positions are for the center of note heads
////  or rests. The last position is for the barline (if exists).
////  This object is responsible for supplying all valid timepos and their positions so
////  that other objects (in fact only lmBoxSlice) could:
////      a) Determine the timepos to assign to a mouse click in a certain position.
////      b) Draw a grid of valid timepos
////
//// tests:
////      + empty_score_builds_empty_table
////      + just_barline_creates_one_entry
////      + one_note_no_barline_creates_two_entries
////      + three_consecutive_notes_creates_four_entries
////      + one_chord_and_barline_creates_two_entries
////      + when_two_notes_at_same_time_choose_the_shortest_one
////      + interpolate_missing_time_between_two_notes
////      + several_lines_with_different_durations
////
////
////-------------------------------------------------------------------------------------
//
//class lmTimeGridTableTestFixture
//{
//public:
//
//    wxSize m_ScoreSize;
//    double m_rScale;
//    lmScore* m_pScore;
//    ScoreLayouter* m_pScoreLayouter;
//    lmBoxScore* m_pBoxScore;
//    lmTimeGridTable* m_pTable;
//
//
//    void load_score_for_test(const wxString& sFilename)
//    {
//        delete_test_data();
//        wxString sPath = g_pPaths->GetTestScoresPath();
//        wxFileName oFilename(sPath, sFilename, "lms"), wxPATH_NATIVE);
//        lmLDPParser parser;
//        m_pScore = parser.ParseFile( oFilename.GetFullPath() );
//        CHECK( m_pScore != NULL );
//
//        lmAggDrawer* pDrawer = new lmAggDrawer(m_ScoreSize.x, m_ScoreSize.y, m_rScale);
//        lmPaper m_oPaper;
//        m_oPaper.SetDrawer(pDrawer);
//        m_pScoreLayouter = new ScoreLayouter(&m_oPaper);
//        m_pBoxScore = m_pScore->Layout(&m_oPaper, m_pScoreLayouter);
//        lmSystemScoreLayouter* pSysFmt = (lmSystemScoreLayouter*) m_pScoreLayouter->GetSystemScoreLayouter(0);
//        lmColumnStorage* pColStorage = pSysFmt->GetColumnData(0);
//        m_pTable = new lmTimeGridTable(pColStorage);
//
//        //wxLogMessage( sFilename );
//        //wxLogMessage( m_pTable->Dump() );
//    }
//
//    void delete_test_data()
//    {
//        if (m_pScore)
//        {
//            delete m_pScore;
//            m_pScore = (lmScore*)NULL;
//        }
//        if (m_pScoreLayouter)
//        {
//            delete m_pScoreLayouter;
//            m_pScoreLayouter = (ScoreLayouter*)NULL;
//        }
//        if (m_pBoxScore)
//        {
//            delete m_pBoxScore;
//            m_pBoxScore = (lmBoxScore*)NULL;
//        }
//        if (m_pTable)
//        {
//            delete m_pTable;
//            m_pTable = (lmTimeGridTable*)NULL;
//        }
//    }
//
//    // setUp
//    lmTimeGridTableTestFixture()
//    {
//        m_ScoreSize = wxSize(700, 1000);
//        m_rScale = 1.0f * lmSCALE;
//        m_pScore = (lmScore*)NULL;
//        m_pScoreLayouter = (ScoreLayouter*)NULL;
//        m_pBoxScore = (lmBoxScore*)NULL;
//        m_pTable = (lmTimeGridTable*)NULL;
//    }
//
//    // tearDown
//    ~lmTimeGridTableTestFixture()
//    {
//        delete_test_data();
//    }
//
//};
//
//
//
//SUITE(lmTimeGridTableTest)
//{
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, empty_score_builds_empty_table)
//    {
//        load_score_for_test("00010-empty-renders-one-staff");
//        CHECK( m_pTable->GetSize() == 0 );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, just_barline_creates_one_entry)
//    {
//        load_score_for_test("90003-empty-bar-with-barline");
//        CHECK( m_pTable->GetSize() == 1 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, one_note_no_barline_creates_two_entries)
//    {
//        load_score_for_test("00022-spacing-in-prolog-one-note");
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 64.0f );
//        CHECK( m_pTable->GetTimepos(1) == 64.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, three_consecutive_notes_creates_four_entries)
//    {
//        load_score_for_test("00023-same-duration-notes-equally-spaced");
//        CHECK( m_pTable->GetSize() == 4 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 64.0f );
//        CHECK( m_pTable->GetTimepos(1) == 64.0f );
//        CHECK( m_pTable->GetDuration(1) == 64.0f );
//        CHECK( m_pTable->GetTimepos(2) == 128.0f );
//        CHECK( m_pTable->GetDuration(2) == 64.0f );
//        CHECK( m_pTable->GetTimepos(3) == 192.0f );
//        CHECK( m_pTable->GetDuration(3) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, one_chord_and_barline_creates_two_entries)
//    {
//        load_score_for_test("00030-chord-notes-are-aligned");
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 256.0f );
//        CHECK( m_pTable->GetTimepos(1) == 256.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, when_two_notes_at_same_time_choose_the_shortest_one)
//    {
//        load_score_for_test("90001-two-notes-different-duration");
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 32.0f );
//        CHECK( m_pTable->GetTimepos(1) == 32.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, interpolate_missing_time_between_two_notes)
//    {
//        load_score_for_test("90004-two-voices-missing-timepos");
//        CHECK( m_pTable->GetSize() == 4 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 32.0f );
//        CHECK( m_pTable->GetTimepos(1) == 32.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        CHECK( m_pTable->GetTimepos(2) == 64.0f );
//        CHECK( m_pTable->GetDuration(2) == 64.0f );
//        CHECK( m_pTable->GetTimepos(3) == 128.0f );
//        CHECK( m_pTable->GetDuration(3) == 0.0f );
//        delete_test_data();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, several_lines_with_different_durations)
//    {
//        load_score_for_test("90002-several-lines-with-different-durations");
//        CHECK( m_pTable->GetSize() == 5 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 64.0f );
//        CHECK( m_pTable->GetTimepos(1) == 64.0f );
//        CHECK( m_pTable->GetDuration(1) == 32.0f );
//        CHECK( m_pTable->GetTimepos(2) == 96.0f );
//        CHECK( m_pTable->GetDuration(2) == 16.0f );
//        CHECK( m_pTable->GetTimepos(3) == 112.0f );
//        CHECK( m_pTable->GetDuration(3) == 16.0f );
//        CHECK( m_pTable->GetTimepos(4) == 128.0f );
//        CHECK( m_pTable->GetDuration(4) == 0.0f );
//        delete_test_data();
//    }

};
