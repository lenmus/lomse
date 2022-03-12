//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
#include "lomse_document_iterator.h"
#include "lomse_ldp_compiler.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class DocIteratorTestFixture
{
public:

    DocIteratorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~DocIteratorTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
};

SUITE(DocIteratorTest)
{
    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorPointsToFirst)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        CHECK( (*it)->is_score() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorNext)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        CHECK( (*it)->is_score() == true );
        ++it;
        CHECK( (*it)->is_score_text() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorNextRemainsAtEnd)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        ++it;
        CHECK( (*it)->is_score_text() == true );
        ++it;
        CHECK( *it == nullptr );
        ++it;
        CHECK( *it == nullptr );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorStartOfContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        ++it;
        ++it;
        ++it;
        CHECK( *it == nullptr );
        it.start_of_content();
        CHECK( (*it)->is_score() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorLastOfContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        it.last_of_content();
        CHECK( (*it)->is_score_text() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorPrev)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        it.last_of_content();
        --it;
        CHECK( (*it)->is_score() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorPrevRemainsAtStart)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        --it;
        CHECK( *it == nullptr );
        --it;
        CHECK( *it == nullptr );
    }

    //TEST_FIXTURE(DocIteratorTestFixture, DocIteratorEnterElement)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
    //    DocIterator it(&doc);
    //    it.enter_element();
    //    CHECK( (*it)->is_instrument() == true );
    //}

//    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorExitElement)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        it.exit_element();
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//        it.exit_element();
//        CHECK( (*it)->to_string() == "(content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\"))" );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorNext)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q))))" );
//        it.enter_element();
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(vers 1.6)" );
//        ++it;
//        CHECK( (*it)->to_string() == "(opt StaffLines.Truncate 1)" );
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        ++it;
//        CHECK( *it == nullptr );
//        ++it;
//        CHECK( *it == nullptr );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorPrev)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        ++it;
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        --it;
//        CHECK( (*it)->to_string() == "(opt StaffLines.Truncate 1)" );
//        --it;
//        CHECK( (*it)->to_string() == "(vers 1.6)" );
//        --it;
//        CHECK( *it == nullptr );
//        --it;
//        CHECK( *it == nullptr );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorPointToType)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.point_to(k_instrument);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        it.point_to(k_instrument);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        it.point_to(k_vers);
//        CHECK( it.is_out_of_range() );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorFindInstrument)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q))) (instrument (musicData (n a3 e)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.find_instrument(0);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        it.find_instrument(1);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n a3 e)))" );
//        it.find_instrument(2);
//        CHECK( *it == nullptr );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorStartOfInstrument)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q))) (instrument (musicData (n a3 e)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.start_of_instrument(0);
//        CHECK( (*it)->to_string() == "(n c4 q)" );
//        it.start_of_instrument(1);
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(n a3 e)" );
//        it.start_of_instrument(2);
//        CHECK( *it == nullptr );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorIncrement)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.start_of_instrument(0);
//        ++it;
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(r q)" );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorDecrement)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.Truncate 1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.start_of_instrument(0);
//        ++it;
//        --it;
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(n c4 q)" );
//    }

}

