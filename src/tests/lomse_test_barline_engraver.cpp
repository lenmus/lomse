//----------------------------------------------------------------------------------------
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
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_barline_engraver.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class BarlineEngrvTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    BarlineEngrvTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~BarlineEngrvTestFixture()    //TearDown fixture
    {
    }
};

SUITE(BarlineEngrvTest)
{

    //TEST_FIXTURE(BarlineEngrvTestFixture, DPitch_C4)
    //{
    //    DiatonicPitch dp(0, 4);
    //    CHECK( (int)dp == LOMSE_C4_DPITCH );
    //}

}


