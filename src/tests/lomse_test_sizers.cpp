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
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
        m_scores_path = TESTLIB_SCORES_PATH;
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
        GmoBoxScorePage box(nullptr);
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
        GmoBoxScorePage box(nullptr);
        sizer.add_child( new SizerChild(&box) );
        sizer.layout(2100.0f, 2970.0f);
        CHECK( sizer.is_vertical() == false );
        CHECK( box.get_width() == _LOMSE_CAN_GROW );
        CHECK( box.get_height() == 2970.0f );
    }

}


