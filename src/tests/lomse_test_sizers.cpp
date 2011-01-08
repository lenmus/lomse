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
#include "lomse_sizers.h"
#include "lomse_gm_basic.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class SizersTestFixture
{
public:

    SizersTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~SizersTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(SizersTest)
{

    // FlowSizer vertical -------------------------------------------------

    TEST_FIXTURE(SizersTestFixture, SizersTest_FlowVerticalTakesAllWidth)
    {
        FlowSizer sizer(FlowSizer::k_vertical);
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub);
        sizer.add_child( new SizerChild(&box) );
        sizer.layout(2100.0f, 2970.0f);
        CHECK( sizer.is_vertical() == true );
        CHECK( box.get_width() == 2100.0f );
        CHECK( box.get_height() == _LOMSE_CAN_GROW );
    }

    // FlowSizer horizontal -----------------------------------------------

    TEST_FIXTURE(SizersTestFixture, SizersTest_FlowHorizontalTakesAllHeight)
    {
        FlowSizer sizer(FlowSizer::k_horizontal);
        GmoStubScore stub(NULL);
        GmoBoxScorePage box(&stub);
        sizer.add_child( new SizerChild(&box) );
        sizer.layout(2100.0f, 2970.0f);
        CHECK( sizer.is_vertical() == false );
        CHECK( box.get_width() == _LOMSE_CAN_GROW );
        CHECK( box.get_height() == 2970.0f );
    }

}


