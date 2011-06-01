//---------------------------------------------------------------------------------------
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

        ScoreIterator sit(pTable);

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

        ScoreIterator sit(pTable);

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
        ScoreIterator sit(pTable);

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
        ScoreIterator sit(pTable);

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
        ScoreIterator sit(pTable);

        ++sit;
        --sit;

        CHECK( (*sit)->imo_object()->is_note() == true );
        CHECK( sit.is_first() == true );
    }

}

