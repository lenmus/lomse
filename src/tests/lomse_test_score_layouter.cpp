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
#include "lomse_score_layouter.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ScoreLayouterTestFixture
{
public:
    //wxSize m_ScoreSize;
    //double m_rScale;
    //lmScore* m_pScore;
    //wxString m_sTestName;
    //wxString m_sTestNum;
    //ScoreLayouter* m_pScoreLayouter;
    //lmBoxScore* m_pBoxScore;
    //lmSystemScoreLayouter* m_pSysFmt;

    ////void SaveScoreBitmap(lmGraphicManager* pGM)
    ////{
    ////    wxBitmap* pBitmap = pGM->RenderScore(1);
    ////    wxString sPath = g_pPaths->GetTestScoresPath();
    ////    wxFileName oFilename(sPath, _T("zimg-")+m_sTestName, _T("bmp"), wxPATH_NATIVE);
    ////    pBitmap->SaveFile(oFilename.GetFullPath(), wxBITMAP_TYPE_BMP);
    ////}

    //void LoadScoreForTest(const wxString& sTestNum, const wxString& sTestName)
    //{
    //    DeleteTestData();
    //    m_sTestNum = sTestNum;
    //    m_sTestName = sTestName;
    //    wxString sPath = g_pPaths->GetTestScoresPath();
    //    wxString sFilename = m_sTestNum + _T("-") + m_sTestName;
    //    wxFileName oFilename(sPath, sFilename, _T("lms"), wxPATH_NATIVE);
    //    lmLDPParser parser;
    //    m_pScore = parser.ParseFile( oFilename.GetFullPath() );
    //    CHECK( m_pScore != NULL );

    //    lmAggDrawer* pDrawer = new lmAggDrawer(m_ScoreSize.x, m_ScoreSize.y, m_rScale);
    //    lmPaper m_oPaper;
    //    m_oPaper.SetDrawer(pDrawer);
    //    m_pScoreLayouter = new ScoreLayouter(&m_oPaper);
    //    m_pBoxScore = m_pScore->Layout(&m_oPaper, m_pScoreLayouter);
    //    m_pSysFmt = m_pScoreLayouter->GetSystemScoreLayouter(0);
    //}

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
    //    wxString sInFile = wxString::Format(_T("ref-%s-%d-%d-%s"),
    //                            m_sTestNum.c_str(), iSys, iCol, m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sInFile, _T("txt"), wxPATH_NATIVE);
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
    //    wxString sActualData = _T("");

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
    //    wxString sInFile = wxString::Format(_T("ref-%s-%s"),
    //                                        m_sTestNum.c_str(), m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sInFile, _T("txt"), wxPATH_NATIVE);
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
    //    wxString sOutFile = wxString::Format(_T("dat-%s-%d-%d-%s"),
    //                                            m_sTestNum.c_str(),
    //                                            iSys, iCol, m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sOutFile, _T("txt"), wxPATH_NATIVE);
    //    wxFile oFile;
    //    oFile.Create(oFilename.GetFullPath(), true);    //true=overwrite
    //    oFile.Open(oFilename.GetFullPath(), wxFile::write);
    //    if (!oFile.IsOpened())
    //    {
    //        wxLogMessage(_T("[ScoreLayouterTest::SaveAsActualData] File '%s' could not be openned. Write to file cancelled"),
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
    //    wxString sOutFile = wxString::Format(_T("dat-%s-%s"),
    //                                         m_sTestNum.c_str(),
    //                                         m_sTestName.c_str() );
    //    wxFileName oFilename(sPath, sOutFile, _T("txt"), wxPATH_NATIVE);
    //    wxFile oFile;
    //    oFile.Create(oFilename.GetFullPath(), true);    //true=overwrite
    //    oFile.Open(oFilename.GetFullPath(), wxFile::write);
    //    if (!oFile.IsOpened())
    //    {
    //        wxLogMessage(_T("[ScoreLayouterTest::SaveAsActualScoreData] File '%s' could not be openned. Write to file cancelled"),
    //            oFilename.GetFullPath().c_str());
    //    }
    //    else
    //    {
    //        oFile.Write(sActualData);
    //        oFile.Close();
    //    }
    //}

    //void DeleteTestData()
    //{
    //    if (m_pScore)
    //    {
    //        delete m_pScore;
    //        m_pScore = (lmScore*)NULL;
    //    }
    //    if (m_pScoreLayouter)
    //    {
    //        delete m_pScoreLayouter;
    //        m_pScoreLayouter = (ScoreLayouter*)NULL;
    //    }
    //    if (m_pBoxScore)
    //    {
    //        delete m_pBoxScore;
    //        m_pBoxScore = (lmBoxScore*)NULL;
    //    }
    //}

    ScoreLayouterTestFixture()   // setUp()
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_pLdpFactory = m_pLibraryScope->ldp_factory();
        m_scores_path = LOMSE_TEST_SCORES_PATH;

        //m_ScoreSize = wxSize(700, 1000);
        //m_rScale = 1.0f * lmSCALE;
        //m_pScore = (lmScore*)NULL;
        //m_pScoreLayouter = (ScoreLayouter*)NULL;
        //m_pBoxScore = (lmBoxScore*)NULL;
    }

    ~ScoreLayouterTestFixture()  // tearDown()
    {
        delete m_pLibraryScope;
        //DeleteTestData();
    }

    LibraryScope* m_pLibraryScope;
    LdpFactory* m_pLdpFactory;
    std::string m_scores_path;
};

SUITE(ScoreLayouterTest)
{

    TEST_FIXTURE(ScoreLayouterTestFixture, ScoreLayouter_ReturnsGModel)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        InternalModel* pIModel = doc.get_im_model();
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoContent* pContent = pDoc->get_content();
        ScoreLayouter scl( pContent->get_item(0) );
        GmoBox* pContainerBox = NULL;
        scl.do_layout(pContainerBox);
        //GraphicModel* pGModel = fmt.get_gm_model();
        //CHECK( pGModel != NULL );
        //delete pGModel;
    }

//            //empty scores
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00010_EmptyScoreRendersOneStaff)
//    {
//        //an empty score with no options only renders one staff
//        LoadScoreForTest(_T("00010"), _T("empty-renders-one-staff"));
//        CHECK( m_pSysFmt->GetNumObjectsInColumnLine(0, 0) == 1 );
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00011_EmptyScoreFillPage)
//    {
//        //an empty score with fill option renders a page full of staves (manuscript paper)
//
//        LoadScoreForTest(_T("00011"), _T("empty-fill-page"));
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
//        DeleteTestData();
//    }
//
//        //spacing a single line
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00020_SpaceBeforeClef)
//    {
//        LoadScoreForTest(_T("00020"), _T("space-before-clef"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00021_SpacingInProlog)
//    {
//        LoadScoreForTest(_T("00021"), _T("spacing-in-prolog"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00022_SpaceAfterPrologOneNote)
//    {
//        LoadScoreForTest(_T("00022"), _T("spacing-in-prolog-one-note"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00023_SameDurationNotesEquallySpaced)
//    {
//        LoadScoreForTest(_T("00023"), _T("same-duration-notes-equally-spaced"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00024_NotesSpacingProportionalToNotesDuration)
//    {
//        LoadScoreForTest(_T("00024"), _T("notes-spacing-proportional-to-notes-duration"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00025_AccidentalsDoNotAlterSpacing)
//    {
//        LoadScoreForTest(_T("00025"), _T("accidentals-do-no-alter-spacing"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00026_AccidentalsDoNotAlterFixedSpacing)
//    {
//        LoadScoreForTest(_T("00026"), _T("accidentals-do-no-alter-fixed-spacing"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00027_Spacing_notes_with_figured_bass)
//    {
//        LoadScoreForTest(_T("00027"), _T("spacing-notes-with-figured-bass"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00030_ChordNotesAreAligned)
//    {
//        LoadScoreForTest(_T("00030"), _T("chord-notes-are-aligned"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00031_ChordStemUpNoteReversedNoFlag)
//    {
//        LoadScoreForTest(_T("00031"), _T("chord-stem-up-note-reversed-no-flag"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00032_ChordStemDownNoteReversedNoFlag)
//    {
//        LoadScoreForTest(_T("00032"), _T("chord-stem-down-note-reversed-no-flag"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00033_ChordsWithReversedNotesDoNotOverlap)
//    {
//        LoadScoreForTest(_T("00033"), _T("chords-with-reversed-notes-do-not-overlap"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00034_ChordWithAccidentalsAligned)
//    {
//        LoadScoreForTest(_T("00034"), _T("chord-with-accidentals-aligned"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00038_ChordsWithAccidentalsAndReversedNotesAligned)
//    {
//        LoadScoreForTest(_T("00038"), _T("chords-with-accidentals-and-reversed-notes-aligned"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00040_ClefBetweenNotesProperlySpacedWhenEnoughSpace)
//    {
//        LoadScoreForTest(_T("00040"), _T("clef-between-notes-properly-spaced-when-enough-space"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00041_ClefBetweenNotesProperlySpacedWhenRemovingVariableSpace)
//    {
//        LoadScoreForTest(_T("00041"), _T("clef-between-notes-properly-spaced-when-removing-variable-space"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00042_ClefBetweenNotesAddsLittleSpacedWhenNotEnoughSpace)
//    {
//        LoadScoreForTest(_T("00042"), _T("clef-between-notes-adds-little-space-when-not-enough-space"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//        // vertical alignment
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00101_VerticalRightAlignmentPrologOneNote)
//    {
//        LoadScoreForTest(_T("00101"), _T("vertical-right-alignment-prolog-one-note"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00102_VerticalRightAlignmentSameTimePositions)
//    {
//        LoadScoreForTest(_T("00102"), _T("vertical-right-alignment-same-time-positions"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00103_VerticalRightAlignmentDifferentTimePositions)
//    {
//        LoadScoreForTest(_T("00103"), _T("vertical-right-alignment-different-time-positions"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    //TEST_FIXTURE(ScoreLayouterTestFixture, T00104_VerticalRightAlignmentWhenAccidentalRequiresMoreSpace)
//    //{
//    //    LoadScoreForTest(_T("00104"), _T("vertical-right-alignment-when-accidental-requires-more-space"));
//    //    LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//    //    DeleteTestData();
//    //}
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00105_VerticalRightAlignmentWhenClefsBetweenNotes)
//    {
//        LoadScoreForTest(_T("00105"), _T("vertical-right-alignment-when-clefs-between-notes"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00106_ClefFollowsNoteWhenNoteDisplaced)
//    {
//        LoadScoreForTest(_T("00106"), _T("clef-follows-note-when-note-displaced"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00107_PrologProperlyAlignedInSecondSystem)
//    {
//        LoadScoreForTest(_T("00107"), _T("prolog-properly-aligned-in-second-system"));
//        LM_ASSERT_LINE_DATA_EQUAL(1, 0);
//        DeleteTestData();
//    }
//
//        //Gourlays' algorithm spacing problems
//        //CPPUNIT_TEST( T00110_triplet_against_5_tuplet_4_14 );
//        //CPPUNIT_TEST( T00111_loose_spacing_4_16 );
//
//    //TEST_FIXTURE(ScoreLayouterTestFixture, T00110_triplet_against_5_tuplet_4_14)
//    //{
//    //    LoadScoreForTest(_T("00110"), _T("triplet-against-5-tuplet-4.14"));
//    //    LM_ASSERT_SCORE_DATA_EQUAL();
//    //    DeleteTestData();
//    //}
//
//        // systems justification (lmLineResizer object)
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00200_BarsGoOneAfterTheOther)
//    {
//        LoadScoreForTest(_T("00200"), _T("bars-go-one-after-the-other"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00201_SystemsAreJustified)
//    {
//        LoadScoreForTest(_T("00201"), _T("systems-are-justified"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(0, 1);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00202_LongSingleBarIsSplitted)
//    {
//        LoadScoreForTest(_T("00202"), _T("long-single-bar-is-splitted"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        LM_ASSERT_LINE_DATA_EQUAL(1, 0);
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00203_repositioning_at_justification)
//    {
//        LoadScoreForTest(_T("00203"), _T("repositioning-at-justification"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//        //other
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T00000_ErrorAlignRests)
//    {
//        LoadScoreForTest(_T("00000"), _T("error-align-rests"));
//        LM_ASSERT_LINE_DATA_EQUAL(0, 0);
//        DeleteTestData();
//    }
//
//        //Regression tests ---------------------------------------------------
//        // not used to drive development. Consider them 'regression tests'
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80010_accidental_after_barline)
//    {
//        LoadScoreForTest(_T("80010"), _T("accidental-after-barline"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80011_accidentals)
//    {
//        LoadScoreForTest(_T("80011"), _T("accidentals"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80020_chord_no_stem_no_flag)
//    {
//        LoadScoreForTest(_T("80020"), _T("chord-no-stem-no-flag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80021_chord_stem_up_no_flag)
//    {
//        LoadScoreForTest(_T("80021"), _T("chord-stem-up-no-flag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80022_chord_stem_down_no_flag)
//    {
//        LoadScoreForTest(_T("80022"), _T("chord-stem-down-no-flag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80023_chord_stem_up_note_reversed_no_flag)
//    {
//        LoadScoreForTest(_T("80023"), _T("chord-stem-up-note-reversed-no-flag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80024_chord_stem_down_note_reversed_no_flag)
//    {
//        LoadScoreForTest(_T("80024"), _T("chord-stem-down-note-reversed-no-flag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80025_chord_stem_up_no_flag_accidental)
//    {
//        LoadScoreForTest(_T("80025"), _T("chord-stem-up-no-flag-accidental"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80026_chord_flags)
//    {
//        LoadScoreForTest(_T("80026"), _T("chord-flags"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80027_chord_spacing)
//    {
//        LoadScoreForTest(_T("80027"), _T("chord-spacing"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80028_chord_notes_ordering)
//    {
//        LoadScoreForTest(_T("80028"), _T("chord-notes-ordering"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80030_tuplet_triplets)
//    {
//        LoadScoreForTest(_T("80030"), _T("tuplet-triplets"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80031_tuplet_duplets)
//    {
//        LoadScoreForTest(_T("80031"), _T("tuplet-duplets"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80032_tuplet_tuplet)
//    {
//        LoadScoreForTest(_T("80032"), _T("tuplet-tuplet"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80040_beams)
//    {
//        LoadScoreForTest(_T("80040"), _T("beams"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80041_chords_beamed)
//    {
//        LoadScoreForTest(_T("80041"), _T("chords-beamed"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80042_beams)
//    {
//        LoadScoreForTest(_T("80042"), _T("beams"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80043_beam_4s_q)
//    {
//        LoadScoreForTest(_T("80043"), _T("beam-4s-q"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80050_ties)
//    {
//        LoadScoreForTest(_T("80050"), _T("ties"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80051_tie_bezier)
//    {
//        LoadScoreForTest(_T("80051"), _T("tie-bezier"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80052_tie_bezier_break)
//    {
//        LoadScoreForTest(_T("80052"), _T("tie-bezier-break"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80053_tie_bezier_barline)
//    {
//        LoadScoreForTest(_T("80053"), _T("tie-bezier-barline"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80054_tie_after_barline)
//    {
//        LoadScoreForTest(_T("80054"), _T("tie-after-barline"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80060_go_back)
//    {
//        LoadScoreForTest(_T("80060"), _T("go-back"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80070_some_time_signatures)
//    {
//        LoadScoreForTest(_T("80070"), _T("some-time-signatures"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80071_12_8_time_signature)
//    {
//        LoadScoreForTest(_T("80071"), _T("12-8-time-signature"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80072_2_4_time_signature)
//    {
//        LoadScoreForTest(_T("80072"), _T("2-4-time-signature"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80080_one_instr_2_staves)
//    {
//        LoadScoreForTest(_T("80080"), _T("one-instr-2-staves"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80081_two_instr_3_staves)
//    {
//        LoadScoreForTest(_T("80081"), _T("two-instr-3-staves"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80082_choir_STB_piano)
//    {
//        LoadScoreForTest(_T("80082"), _T("choir-STB-piano"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80083_critical_line)
//    {
//        LoadScoreForTest(_T("80083"), _T("critical-line"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80090_all_rests)
//    {
//        LoadScoreForTest(_T("80090"), _T("all-rests"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80091_rests_in_beam)
//    {
//        LoadScoreForTest(_T("80091"), _T("rests-in-beam"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80092_short_rests_in_beam)
//    {
//        LoadScoreForTest(_T("80092"), _T("short-rests-in-beam"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80100_spacer)
//    {
//        LoadScoreForTest(_T("80100"), _T("spacer"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80110_graphic_line_text)
//    {
//        LoadScoreForTest(_T("80110"), _T("graphic-line-text"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80111_line_after_barline)
//    {
//        LoadScoreForTest(_T("80111"), _T("line-after-barline"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80120_fermatas)
//    {
//        LoadScoreForTest(_T("80120"), _T("fermatas"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80130_metronome)
//    {
//        LoadScoreForTest(_T("80130"), _T("metronome"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80131_metronome)
//    {
//        LoadScoreForTest(_T("80131"), _T("metronome"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80132_metronome)
//    {
//        LoadScoreForTest(_T("80132"), _T("metronome"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80140_text)
//    {
//        LoadScoreForTest(_T("80140"), _T("text"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80141_text_titles)
//    {
//        LoadScoreForTest(_T("80141"), _T("text-titles"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80150_all_clefs)
//    {
//        LoadScoreForTest(_T("80150"), _T("all-clefs"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80151_all_clefs)
//    {
//        LoadScoreForTest(_T("80151"), _T("all-clefs"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80160_textbox)
//    {
//        LoadScoreForTest(_T("80160"), _T("textbox"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80161_textbox_with_anchor_line)
//    {
//        LoadScoreForTest(_T("80161"), _T("textbox-with-anchor-line"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80162_stacked_textboxes)
//    {
//        LoadScoreForTest(_T("80162"), _T("stacked-textboxes"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80170_figured_bass)
//    {
//        LoadScoreForTest(_T("80170"), _T("figured-bass"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80171_figured_bass_several)
//    {
//        LoadScoreForTest(_T("80171"), _T("figured-bass-several"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80172_figured_bass_line)
//    {
//        LoadScoreForTest(_T("80172"), _T("figured-bass-line"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(ScoreLayouterTestFixture, T80180_new_system_tag)
//    {
//        LoadScoreForTest(_T("80180"), _T("new-system-tag"));
//        LM_ASSERT_SCORE_DATA_EQUAL();
//        DeleteTestData();
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
//    void LoadScoreForTest(const wxString& sFilename)
//    {
//        DeleteTestData();
//        wxString sPath = g_pPaths->GetTestScoresPath();
//        wxFileName oFilename(sPath, sFilename, _T("lms"), wxPATH_NATIVE);
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
//    void DeleteTestData()
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
//        DeleteTestData();
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
//        LoadScoreForTest(_T("00010-empty-renders-one-staff"));
//        CHECK( m_pTable->GetSize() == 0 );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, just_barline_creates_one_entry)
//    {
//        LoadScoreForTest(_T("90003-empty-bar-with-barline"));
//        CHECK( m_pTable->GetSize() == 1 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, one_note_no_barline_creates_two_entries)
//    {
//        LoadScoreForTest(_T("00022-spacing-in-prolog-one-note"));
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 64.0f );
//        CHECK( m_pTable->GetTimepos(1) == 64.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, three_consecutive_notes_creates_four_entries)
//    {
//        LoadScoreForTest(_T("00023-same-duration-notes-equally-spaced"));
//        CHECK( m_pTable->GetSize() == 4 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 64.0f );
//        CHECK( m_pTable->GetTimepos(1) == 64.0f );
//        CHECK( m_pTable->GetDuration(1) == 64.0f );
//        CHECK( m_pTable->GetTimepos(2) == 128.0f );
//        CHECK( m_pTable->GetDuration(2) == 64.0f );
//        CHECK( m_pTable->GetTimepos(3) == 192.0f );
//        CHECK( m_pTable->GetDuration(3) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, one_chord_and_barline_creates_two_entries)
//    {
//        LoadScoreForTest(_T("00030-chord-notes-are-aligned"));
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 256.0f );
//        CHECK( m_pTable->GetTimepos(1) == 256.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, when_two_notes_at_same_time_choose_the_shortest_one)
//    {
//        LoadScoreForTest(_T("90001-two-notes-different-duration"));
//        CHECK( m_pTable->GetSize() == 2 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 32.0f );
//        CHECK( m_pTable->GetTimepos(1) == 32.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, interpolate_missing_time_between_two_notes)
//    {
//        LoadScoreForTest(_T("90004-two-voices-missing-timepos"));
//        CHECK( m_pTable->GetSize() == 4 );
//        CHECK( m_pTable->GetTimepos(0) == 0.0f );
//        CHECK( m_pTable->GetDuration(0) == 32.0f );
//        CHECK( m_pTable->GetTimepos(1) == 32.0f );
//        CHECK( m_pTable->GetDuration(1) == 0.0f );
//        CHECK( m_pTable->GetTimepos(2) == 64.0f );
//        CHECK( m_pTable->GetDuration(2) == 64.0f );
//        CHECK( m_pTable->GetTimepos(3) == 128.0f );
//        CHECK( m_pTable->GetDuration(3) == 0.0f );
//        DeleteTestData();
//    }
//
//    TEST_FIXTURE(lmTimeGridTableTestFixture, several_lines_with_different_durations)
//    {
//        LoadScoreForTest(_T("90002-several-lines-with-different-durations"));
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
//        DeleteTestData();
//    }

};
