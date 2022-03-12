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
#include "lomse_ldp_factory.h"
#include "private/lomse_document_p.h"
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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
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
    //    CHECK( (int)dp == C4_DPITCH );
    //}

}


