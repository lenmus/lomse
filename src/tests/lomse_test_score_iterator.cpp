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

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_staffobjs_table.h"
#include "lomse_score_iterator.h"
#include "lomse_internal_model.h"
//#include "lomse_ldp_elements.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class ScoreIteratorTestFixture
{
public:

    ScoreIteratorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ScoreIteratorTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
    LdpFactory* m_pLdpFactory;

};

SUITE(ScoreIteratorTest)
{

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorPointsFirst)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        CHECK( (*it)->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != NULL );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_entries() == 2 );

        StaffObjsIterator sit(pTable);

        CHECK( (*sit)->imo_object()->is_note() == true );
        CHECK( (*sit)->line() == 0 );
        CHECK( (*sit)->measure() == 0 );
        CHECK( (*sit)->time() == 0.0f );
        CHECK( (*sit)->num_instrument() == 0 );
        CHECK( (*sit)->staff() == 0 );
    }


    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorIsFirst)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        CHECK( (*it)->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != NULL );
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        CHECK( pTable->num_entries() == 2 );

        StaffObjsIterator sit(pTable);

        CHECK( sit.is_first() == true );
        CHECK( sit.is_end() == false );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorNext)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);

        ++sit;

        CHECK( (*sit)->imo_object()->is_rest() == true );
        CHECK( (*sit)->line() == 0 );
        CHECK( (*sit)->measure() == 0 );
        CHECK( (*sit)->time() == 64.0f );
        CHECK( (*sit)->num_instrument() == 0 );
        CHECK( (*sit)->staff() == 0 );
        CHECK( sit.is_first() == false );
        CHECK( sit.is_end() == false );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorEndOfTable)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);

        ++sit;
        ++sit;

        CHECK( sit.is_first() == false );
        CHECK( sit.is_end() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorPrev)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);

        ++sit;
        --sit;

        CHECK( (*sit)->imo_object()->is_note() == true );
        CHECK( sit.is_first() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_prev_0)
    {
        //@ prev() returns NULL if at start
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);

        CHECK( sit.prev() == NULL );
        CHECK( (*sit)->imo_object()->is_note() == true );
        CHECK( sit.is_first() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_prev_1)
    {
        //@ prev() returns prev object at intermediate position
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ColStaffObjsEntry* pEntry = *sit;
        ++sit;

        CHECK( sit.prev() == pEntry );
        CHECK( (*sit)->imo_object()->is_rest() == true );
        CHECK( sit.is_first() == false );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_prev_2)
    {
        //@ prev() returns prev object at end
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ++sit;
        ColStaffObjsEntry* pEntry = *sit;
        ++sit;

        CHECK( sit.prev() == pEntry );
        CHECK( sit.is_end() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_next_0)
    {
        //@ next() returns NULL if at end.
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ++sit;
        ++sit;

        CHECK( sit.next() == NULL );
        CHECK( sit.is_end() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_next_1)
    {
        //@ next() returns next object at start.
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ColStaffObjsEntry* pEntry = sit.next();
        ++sit;

        CHECK( *sit == pEntry );
        CHECK( (*sit)->imo_object()->is_rest() == true );
        CHECK( sit.is_first() == false );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_next_2)
    {
        //@ next() returns next object at start. Moving backwards
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ++sit;
        ++sit;
        CHECK( sit.is_end() == true );

        --sit;
        --sit;
        CHECK( sit.is_first() == true );
        ColStaffObjsEntry* pEntry = sit.next();
        ++sit;
        CHECK( *sit == pEntry );
        CHECK( (*sit)->imo_object()->is_rest() == true );
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_next_3)
    {
        //@ next() returns next object at intermediate position
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef F4) (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        ColStaffObjs* pTable = pScore->get_staffobjs_table();
        StaffObjsIterator sit(pTable);
        ++sit;
        CHECK( (*sit)->imo_object()->is_note() == true );

        ColStaffObjsEntry* pEntry = sit.next();
        ++sit;

        CHECK( *sit == pEntry );
        CHECK( pEntry->imo_object()->is_rest() == true );
    }

}

