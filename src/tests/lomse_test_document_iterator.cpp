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
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_document_iterator.h"
#include "lomse_compiler.h"
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
        CHECK( *it == NULL );
        ++it;
        CHECK( *it == NULL );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorStartOfContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocIterator it(&doc);
        ++it;
        ++it;
        ++it;
        CHECK( *it == NULL );
        it.start_of_content();
        CHECK( (*it)->is_score() == true );
    }

    TEST_FIXTURE(DocIteratorTestFixture, DocIteratorLastOfContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
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
        CHECK( *it == NULL );
        --it;
        CHECK( *it == NULL );
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
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q))))" );
//        it.enter_element();
//        //cout << (*it)->to_string() << endl;
//        CHECK( (*it)->to_string() == "(vers 1.6)" );
//        ++it;
//        CHECK( (*it)->to_string() == "(opt StaffLines.StopAtFinalBarline false)" );
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        ++it;
//        CHECK( *it == NULL );
//        ++it;
//        CHECK( *it == NULL );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorPrev)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        ++it;
//        ++it;
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        --it;
//        CHECK( (*it)->to_string() == "(opt StaffLines.StopAtFinalBarline false)" );
//        --it;
//        CHECK( (*it)->to_string() == "(vers 1.6)" );
//        --it;
//        CHECK( *it == NULL );
//        --it;
//        CHECK( *it == NULL );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorPointToType)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
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
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q))) (instrument (musicData (n a3 e)))) (text \"this is text\")))" );
//        DocIterator it(&doc);
//        ++it;
//        it.enter_element();
//        it.enter_element();
//        it.find_instrument(0);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n c4 q) (r q)))" );
//        it.find_instrument(1);
//        CHECK( (*it)->to_string() == "(instrument (musicData (n a3 e)))" );
//        it.find_instrument(2);
//        CHECK( *it == NULL );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorStartOfInstrument)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q))) (instrument (musicData (n a3 e)))) (text \"this is text\")))" );
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
//        CHECK( *it == NULL );
//    }
//
//    TEST_FIXTURE(DocIteratorTestFixture, ScoreElmIteratorIncrement)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
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
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
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

