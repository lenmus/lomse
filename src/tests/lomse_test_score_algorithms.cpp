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
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document_cursor.h"


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

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_1)
    {
        //inserted note starts and ends at same time than existing note
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

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_2)
    {
        //inserted note starts after and ends at same time than existing note
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

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_3)
    {
        //inserted note starts before and ends at same time than existing note
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

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_4)
    {
        //inserted note starts at same time than existing note, but ends before
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

    TEST_FIXTURE(ScoreAlgorithmsTestFixture, find_and_classify_5)
    {
        //inserted note starts before and ends at same time than existing note
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

}

