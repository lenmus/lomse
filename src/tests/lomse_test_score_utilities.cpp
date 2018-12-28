//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
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
