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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document_cursor.h"
#include "lomse_score_algorithms.h"
#include "lomse_measures_table.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ScoreAlgorithmsTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;


    ScoreAlgorithmsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
    }

    ~ScoreAlgorithmsTestFixture()    //TearDown fixture
    {
    }

    void delete_overlaps(list<OverlappedNoteRest*>& overlaps)
    {
        list<OverlappedNoteRest*>::iterator it = overlaps.begin();
        while (it != overlaps.end())
        {
            OverlappedNoteRest* pOV = *it;
            it = overlaps.erase(it);
            delete pOV;
        }
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

SUITE(ScoreAlgorithmsTest)
{

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_noterest_1)
    {
        //note exists and starts at same timepos
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        ImoNote* pNote = static_cast<ImoNote*>(
                            ScoreAlgorithms::find_noterest_at(pScore, 0, 1, 0.0) );

        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_021)
    {
        //@021. requested interval starts and ends at same time than existing note
        // 1 case: nrT == t (full)
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          0--------32
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        list<OverlappedNoteRest*> overlaps =
            ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
                                                        pScore, 0, 1, 0.0, 32.0);

        OverlappedNoteRest* pOV = overlaps.front();
        ImoNote* pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( overlaps.size() == 1 );
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
        CHECK( pOV->type == k_overlap_full );
        CHECK( is_equal_time(pOV->overlap, 32.0) );

        delete_overlaps(overlaps);
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_022)
    {
        //@022. requested interval starts after and ends at same time than existing note
        // 1 case: nrT < t (at_end)
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //               24--32
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        list<OverlappedNoteRest*> overlaps =
            ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
                                                        pScore, 0, 1, 24.0, 8.0);

        OverlappedNoteRest* pOV = overlaps.front();
        ImoNote* pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( overlaps.size() == 1 );
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
        CHECK( pOV->type == k_overlap_at_end );
        CHECK( is_equal_time(pOV->overlap, 8.0) );

        delete_overlaps(overlaps);
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_023)
    {
        //@023. requested interval starts before and ends at same time than existing note
        // 2 cases: nrT < t (at end) & t < nrT (full)
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //               24-------------64
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        list<OverlappedNoteRest*> overlaps =
            ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
                                                        pScore, 0, 1, 24.0, 40.0);

        CHECK( overlaps.size() == 2 );
        list<OverlappedNoteRest*>::const_iterator it = overlaps.begin();
        OverlappedNoteRest* pOV = *it;
        ImoNote* pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
        CHECK( pOV->type == k_overlap_at_end );
        CHECK( is_equal_time(pOV->overlap, 8.0) );

        ++it;
        pOV = *it;
        pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("f4") );
        CHECK( pOV->type == k_overlap_full );
        CHECK( is_equal_time(pOV->overlap, 32.0) );

        delete_overlaps(overlaps);
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_024)
    {
        //@024. requested interval starts at same time than existing note, but ends before
        // 1 case: nrT == t (at_start)
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //          0----24
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        list<OverlappedNoteRest*> overlaps =
            ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
                                                        pScore, 0, 1, 0.0, 24.0);

        OverlappedNoteRest* pOV = overlaps.front();
        ImoNote* pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( overlaps.size() == 1 );
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
        CHECK( pOV->type == k_overlap_at_start );
        CHECK( is_equal_time(pOV->overlap, 24.0) );

        delete_overlaps(overlaps);
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_025)
    {
        //@025. requested interval starts before and ends at same time than existing note
        //  (clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)
        //               24----------56
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e v1)(n f4 e v1)(n g4 e v1)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        list<OverlappedNoteRest*> overlaps =
            ScoreAlgorithms::find_and_classify_overlapped_noterests_at(
                                                        pScore, 0, 1, 24.0, 32.0);

        CHECK( overlaps.size() == 2 );
        list<OverlappedNoteRest*>::const_iterator it = overlaps.begin();
        OverlappedNoteRest* pOV = *it;
        ImoNote* pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("e4") );
        CHECK( pOV->type == k_overlap_at_end );
        CHECK( is_equal_time(pOV->overlap, 8.0) );

        ++it;
        pOV = *it;
        pNote = static_cast<ImoNote*>(pOV->pNR);
        CHECK( pNote != nullptr );
        CHECK( pNote->get_fpitch() == FPitch("f4") );
        CHECK( pOV->type == k_overlap_at_start );
        CHECK( is_equal_time(pOV->overlap, 24.0) );

        delete_overlaps(overlaps);
    }



    // using the measures table ---------------------------------------------------------

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_001)
    {
        //@001. Found in first guess. At start of measure

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(384.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 384.0f );
        CHECK( pMeasure->get_table_index() == 3 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_002)
    {
        //@002. Found in first guess. At middle

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(400.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 384.0f );
        CHECK( pMeasure->get_table_index() == 3 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_003)
    {
        //@003. First guess is low

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(660.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 640.0f );
        CHECK( pMeasure->get_table_index() == 5 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_004)
    {
        //@004. First guess is high

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(150.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 128.0f );
        CHECK( pMeasure->get_table_index() == 1 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_005)
    {
        //@005. Lowest value

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(0.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 0.0f );
        CHECK( pMeasure->get_table_index() == 0 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, measures_table_006)
    {
        //@006. Higher than max

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImMeasuresTable* pTable = pInstr->get_measures_table();
        CHECK( pTable->num_entries() == 8 );
//        cout << test_name() << endl;
//        cout << pTable->dump();

        ImMeasuresTableEntry* pMeasure = pTable->get_measure_at(1000.0);

        CHECK( pMeasure != nullptr );
        CHECK( pMeasure->get_timepos() == 896.0f );
        CHECK( pMeasure->get_table_index() == 7 );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_locator_for_100)
    {
        //@100. just check that ImMeasuresTable::get_measure_at() is invoked

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))");
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MeasureLocator ml = ScoreAlgorithms::get_locator_for(pScore, 160.0);

//        cout << test_name() << endl;
//        cout << "instr=" << ml.iInstr << ", meas=" << ml.iMeasure << ", loc=" << ml.location << endl;
        CHECK( ml.iInstr == 0 );
        CHECK( ml.iMeasure == 1 );      //measure 1 starts at 128
        CHECK( ml.location == 32.0 );   //160 - 128 = 32
    }


    // get timepos for measure/beat -----------------------------------------------------

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_200)
    {
        //@200. measure too high. Return 0.0.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 22, beat 2
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 21, 1, 0);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_201)
    {
        //@201. measure negative. Return 0.0

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure -4
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, -4, 1, 0);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_202)
    {
        //@202. beat too high. Take as valid

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 2, beat 3
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 2, 0);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 256.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_203)
    {
        //@203. beat negative. Assume beat = 0

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 3, beat -2
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 2, -1, 0);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 256.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_204)
    {
        //@204. instrument negative. Return 0.0

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 3, beat 2
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 2, 1, -3);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_205)
    {
        //@205. instrument too high. Return 0.0
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 3, beat 2
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 2, 1, 3);

//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );
    }

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, get_timepos_for_206)
    {
        //@206. valid parameters return correct values

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        ImoScore* pScore =
            static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        //measure 2, beat 2 ==> 128+64 = 192
        TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 1, 0);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 192.0) );

        //measure 1, beat 1 ==> 0
        timepos = ScoreAlgorithms::get_timepos_for(pScore, 0, 0, 0);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );

        //measure 8, beat 1 ==> 896
        timepos = ScoreAlgorithms::get_timepos_for(pScore, 7, 0, 0);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 896.0) );

        //measure 4, beat 2 ==> 384+64 = 448
        timepos = ScoreAlgorithms::get_timepos_for(pScore, 3, 1, 0);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 448.0) );
    }

}

