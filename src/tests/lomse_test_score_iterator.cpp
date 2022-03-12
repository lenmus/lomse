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

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "private/lomse_document_p.h"
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            CHECK( pTable->num_entries() == 2 );

            StaffObjsIterator sit(pTable);

            CHECK( sit.is_first() == true );
            CHECK( sit.is_end() == false );
        }
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorNext)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != nullptr );
        if (pScore)
        {
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
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorEndOfTable)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);

            ++sit;
            ++sit;

            CHECK( sit.is_first() == false );
            CHECK( sit.is_end() == true );
        }
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorPrev)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);

            ++sit;
            --sit;

            CHECK( (*sit)->imo_object()->is_note() == true );
            CHECK( sit.is_first() == true );
        }
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_prev_0)
    {
        //@ prev() returns nullptr if at start
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);

            CHECK( sit.prev() == nullptr );
            CHECK( (*sit)->imo_object()->is_note() == true );
            CHECK( sit.is_first() == true );
        }
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);
            ColStaffObjsEntry* pEntry = *sit;
            ++sit;

            CHECK( sit.prev() == pEntry );
            CHECK( (*sit)->imo_object()->is_rest() == true );
            CHECK( sit.is_first() == false );
        }
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);
            ++sit;
            ColStaffObjsEntry* pEntry = *sit;
            ++sit;

            CHECK( sit.prev() == pEntry );
            CHECK( sit.is_end() == true );
        }
    }

    TEST_FIXTURE(ScoreIteratorTestFixture, score_iterator_next_0)
    {
        //@ next() returns nullptr if at end.
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        DocIterator it(&doc);
        it.start_of_content();
        ImoScore* pScore = dynamic_cast<ImoScore*>(*it);
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);
            ++sit;
            ++sit;

            CHECK( sit.next() == nullptr );
            CHECK( sit.is_end() == true );
        }
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            StaffObjsIterator sit(pTable);
            ColStaffObjsEntry* pEntry = sit.next();
            ++sit;

            CHECK( *sit == pEntry );
            CHECK( (*sit)->imo_object()->is_rest() == true );
            CHECK( sit.is_first() == false );
        }
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
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
        CHECK( pScore != nullptr );
        if (pScore)
        {
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

}

