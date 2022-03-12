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
#include "lomse_internal_model.h"
#include "lomse_score_utilities.h"
#include "lomse_pitch.h"
#include "lomse_interval.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class ScoreUtilitiesTestFixture
{
public:
    LibraryScope m_libraryScope;

    ScoreUtilitiesTestFixture()   // setUp()
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ScoreUtilitiesTestFixture()  // tearDown()
    {
    }
};


SUITE(ScoreUtilitiesTest)
{

    //@ KeyUtilities --------------------------------------------------------------------

    TEST_FIXTURE(ScoreUtilitiesTestFixture, KeyUtilities_01)
    {
        //@01. Interval between two keys

        CHECK( KeyUtilities::up_interval(k_key_G, k_key_A) == FIntval("M2", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_G, k_key_A) == FIntval("m7", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_G, k_key_A) == FIntval("M2", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_C, k_key_Ef) == FIntval("m3", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_C, k_key_Ef) == FIntval("M6", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_C, k_key_Ef) == FIntval("m3", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_Ef, k_key_C) == FIntval("M6", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_Ef, k_key_C) == FIntval("m3", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_Ef, k_key_C) == FIntval("m3", k_descending) );

        CHECK( KeyUtilities::up_interval(k_key_Cs, k_key_Cf) == FIntval("dd8", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_Cs, k_key_Cf) == FIntval("da1", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_Cs, k_key_Cf) == FIntval("da1", k_descending) );

        CHECK( KeyUtilities::up_interval(k_key_C, k_key_Cs) == FIntval("a1", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_C, k_key_Cs) == FIntval("d8", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_C, k_key_Cs) == FIntval("a1", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_C, k_key_Df) == FIntval("m2", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_C, k_key_Df) == FIntval("M7", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_C, k_key_Df) == FIntval("m2", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_C, k_key_Fs) == FIntval("a4", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_C, k_key_Fs) == FIntval("d5", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_C, k_key_Fs) == FIntval("a4", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_E, k_key_F) == FIntval("m2", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_E, k_key_F) == FIntval("M7", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_E, k_key_F) == FIntval("m2", k_ascending) );

        CHECK( KeyUtilities::up_interval(k_key_D, k_key_Df) == FIntval("d8", k_ascending) );
        CHECK( KeyUtilities::down_interval(k_key_D, k_key_Df) == FIntval("a1", k_descending) );
        CHECK( KeyUtilities::closest_interval(k_key_D, k_key_Df) == FIntval("a1", k_descending) );
    }

};
