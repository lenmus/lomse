//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_score_meter.h"
#include "lomse_document.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ScoreMeterTestFixture
{
public:
    LibraryScope m_libraryScope;

    ScoreMeterTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
    }

    ~ScoreMeterTestFixture()  // tearDown()
    {
    }
};


SUITE(ScoreMeterTest)
{

    TEST_FIXTURE(ScoreMeterTestFixture, ScoreMeter_NumStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2)(musicData ))"
            "(instrument (staves 3)(musicData ))"
            ")))" );
        ImoScore* pScore = doc.get_score();
        ScoreMeter meter(pScore);

        CHECK( meter.num_instruments() == 2 );
        CHECK( meter.num_staves() == 5 );
    }

};
