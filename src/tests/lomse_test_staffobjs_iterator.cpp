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
#include "lomse_internal_model.h"
//#include "lomse_ldp_elements.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class ColStaffObjsIteratorTestFixture
{
public:

    ColStaffObjsIteratorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_pLdpFactory = m_libraryScope.ldp_factory();
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ColStaffObjsIteratorTestFixture()    //TearDown fixture
    {
    }

    LibraryScope m_libraryScope;
    LdpFactory* m_pLdpFactory;

};

SUITE(ColStaffObjsIteratorTest)
{

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, ColStaffObjsIteratorPointsFirst)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            CHECK( pTable->num_entries() == 2 );

            ColStaffObjsIterator it = pTable->begin();

            CHECK( (*it)->imo_object()->is_note() == true );
            CHECK( (*it)->line() == 0 );
            CHECK( (*it)->measure() == 0 );
            CHECK( (*it)->time() == 0.0f );
            CHECK( (*it)->num_instrument() == 0 );
            CHECK( (*it)->staff() == 0 );
        }
    }


    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, ColStaffObjsIteratorIsFirst)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            CHECK( pTable->num_entries() == 2 );

            ColStaffObjsIterator it = pTable->begin();

            CHECK( it == pTable->begin() );
            CHECK( it != pTable->end() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, ColStaffObjsIteratorNext)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();

            ++it;

            CHECK( (*it)->imo_object()->is_rest() == true );
            CHECK( (*it)->line() == 0 );
            CHECK( (*it)->measure() == 0 );
            CHECK( (*it)->time() == 64.0f );
            CHECK( (*it)->num_instrument() == 0 );
            CHECK( (*it)->staff() == 0 );
            CHECK( it != pTable->begin() );
            CHECK( it != pTable->end() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, ColStaffObjsIteratorEndOfTable)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();

            ++it;
            ++it;

            CHECK( it != pTable->begin() );
            CHECK( it == pTable->end() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, ColStaffObjsIteratorPrev)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();

            ++it;
            --it;

            CHECK( (*it)->imo_object()->is_note() == true );
            CHECK( it == pTable->begin() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_prev_0)
    {
        //@ prev() returns nullptr if at start
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();

            CHECK( it.prev() == nullptr );
            CHECK( (*it)->imo_object()->is_note() == true );
            CHECK( it == pTable->begin() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_prev_1)
    {
        //@ prev() returns prev object at intermediate position
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ColStaffObjsEntry* pEntry = *it;
            ++it;

            CHECK( it.prev() == pEntry );
            CHECK( (*it)->imo_object()->is_rest() == true );
            CHECK( it != pTable->begin() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_prev_2)
    {
        //@ prev() returns prev object at end
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ++it;
            ColStaffObjsEntry* pEntry = *it;
            ++it;

            CHECK( it.prev() == pEntry );
            CHECK( it == pTable->end() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_next_0)
    {
        //@ next() returns nullptr if at end.
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ++it;
            ++it;

            CHECK( it.next() == nullptr );
            CHECK( it == pTable->end() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_next_1)
    {
        //@ next() returns next object at start.
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ColStaffObjsEntry* pEntry = it.next();
            ++it;

            CHECK( *it == pEntry );
            CHECK( (*it)->imo_object()->is_rest() == true );
            CHECK( it != pTable->begin() );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_next_2)
    {
        //@ next() returns next object at start. Moving backwards
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ++it;
            ++it;
            CHECK( it == pTable->end() );

            --it;
            --it;
            CHECK( it == pTable->begin() );
            ColStaffObjsEntry* pEntry = it.next();
            ++it;
            CHECK( *it == pEntry );
            CHECK( (*it)->imo_object()->is_rest() == true );
        }
    }

    TEST_FIXTURE(ColStaffObjsIteratorTestFixture, score_iterator_next_3)
    {
        //@ next() returns next object at intermediate position
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef F4) (n c4 q) (r q)) ))))" );
        ImoScore* pScore = dynamic_cast<ImoScore*>(doc.get_first_content_item());
        CHECK( pScore != nullptr );
        if (pScore)
        {
            ColStaffObjs* pTable = pScore->get_staffobjs_table();
            ColStaffObjsIterator it = pTable->begin();
            ++it;
            CHECK( (*it)->imo_object()->is_note() == true );

            ColStaffObjsEntry* pEntry = it.next();
            ++it;

            CHECK( *it == pEntry );
            CHECK( pEntry->imo_object()->is_rest() == true );
        }
    }

}

