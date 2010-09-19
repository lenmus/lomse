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

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <sstream>

//classes related to these tests
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_staffobjs_table.h"
#include "lomse_score_iterator.h"

//to delete singletons
#include "lomse_factory.h"
#include "lomse_elements.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class ScoreIteratorTestFixture
{
public:

    ScoreIteratorTestFixture()     //SetUp fixture
    {
    }

    ~ScoreIteratorTestFixture()    //TearDown fixture
    {
        delete Factory::instance();
    }

};

SUITE(ScoreIteratorTest)
{

    TEST_FIXTURE(ScoreIteratorTestFixture, ScoreIteratorEmptyScore)
    {
        Document doc;
        doc.from_string("(lenmusdoc (vers 0.0) (content ))" );
        DocIterator it(&doc);
        it.start_of_content();  //points to score
        ScoreIterator table(&doc, it.get_iterator());
        CHECK( table.num_entries() == 0 );
    }

}

#endif  // _LM_DEBUG_

