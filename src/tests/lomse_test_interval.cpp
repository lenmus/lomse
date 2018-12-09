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
#include "lomse_interval.h"
#include "lomse_pitch.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class IntervalTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    IntervalTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~IntervalTestFixture()    //TearDown fixture
    {
    }
};

SUITE(IntervalTest)
{

    TEST_FIXTURE(IntervalTestFixture, fintval_01)
    {
        //@01. Constructor from components, simple intervals

        FIntval intvl1(2, k_major);
        CHECK( int(intvl1) == k_interval_M2 );
        FIntval intvl2(7, k_augmented);
        CHECK( int(intvl2) == k_interval_a7 );
        FIntval intvl3(8, k_double_augmented);
        CHECK( int(intvl3) == k_interval_da8 );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_02)
    {
        //@02. Constructor from components, compound intervals, up to two octaves

        FIntval intvl1(9, k_major);
        CHECK( int(intvl1) == k_interval_M2 + k_interval_p8 );
        FIntval intvl2(14, k_augmented);
        CHECK( int(intvl2) == k_interval_a7 + k_interval_p8 );
        FIntval intvl3(15, k_perfect);
        CHECK( int(intvl3) == k_interval_p8 + k_interval_p8 );
        FIntval intvl4(12, k_perfect);
        CHECK( int(intvl4) == k_interval_p5 + k_interval_p8 );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_03)
    {
        //@03. Constructor from components, compound intervals, greater than two octaves

        FIntval intvl1(16, k_major);
        CHECK( int(intvl1) == k_interval_M2 + k_interval_p8 + k_interval_p8 );
        FIntval intvl2(21, k_augmented);
        CHECK( int(intvl2) == k_interval_a7 + k_interval_p8 + k_interval_p8 );
        FIntval intvl3(22, k_perfect);
        CHECK( int(intvl3) == k_interval_p8 + k_interval_p8 + k_interval_p8 );
        FIntval intvl4(19, k_augmented);
        CHECK( int(intvl4) == k_interval_a5 + k_interval_p8 + k_interval_p8 );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_04)
    {
        //@04. Constructor from code

        FIntval intvl1("M2");
        CHECK( int(intvl1) == k_interval_M2 );
        FIntval intvl2("a7");
        CHECK( int(intvl2) == k_interval_a7 );
        FIntval intvl3("p8");
        CHECK( int(intvl3) == k_interval_p8 );
        FIntval intvl4("a5");
        CHECK( int(intvl4) == k_interval_a5 );
        FIntval intvl5("dd6");
        CHECK( int(intvl5) == k_interval_dd6 );
        FIntval intvl6("p15");
        CHECK( int(intvl6) == k_interval_null );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_05)
    {
        //@05. get_code()

        CHECK( FIntval("M2").get_code() == "M2" );
        CHECK( FIntval("da3").get_code() == "da3" );
        CHECK( FIntval("d4").get_code() == "d4" );
        CHECK( FIntval("m6").get_code() == "m6" );
        CHECK( FIntval("p8").get_code() == "p8" );
        CHECK( FIntval(9, k_major).get_code() == "M9" );
        CHECK( FIntval(14, k_augmented).get_code() == "a14" );
        CHECK( FIntval(15, k_perfect).get_code() == "p15" );
        CHECK( FIntval(12, k_perfect).get_code() == "p12" );
        CHECK( FIntval(16, k_major).get_code() == "M16" );
        CHECK( FIntval(21, k_augmented).get_code() == "a21" );
        CHECK( FIntval(22, k_perfect).get_code() == "p22" );
        CHECK( FIntval(19, k_perfect).get_code() == "p19" );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_06)
    {
        //@06. get_number()

        CHECK( FIntval("M2").get_number() == 2 );
        CHECK( FIntval("da3").get_number() == 3 );
        CHECK( FIntval("d4").get_number() == 4 );
        CHECK( FIntval("m6").get_number() == 6 );
        CHECK( FIntval("p8").get_number() == 8 );
        CHECK( FIntval(9, k_major).get_number() == 9 );
        CHECK( FIntval(14, k_augmented).get_number() == 14 );
        CHECK( FIntval(15, k_perfect).get_number() == 15 );
        CHECK( FIntval(12, k_perfect).get_number() == 12 );
        CHECK( FIntval(16, k_major).get_number() == 16 );
        CHECK( FIntval(21, k_augmented).get_number() == 21 );
        CHECK( FIntval(22, k_perfect).get_number() == 22 );
        CHECK( FIntval(19, k_perfect).get_number() == 19 );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_07)
    {
        //@07. get_type()

        CHECK( FIntval("a1").get_type() == k_augmented );
        CHECK( FIntval("M2").get_type() == k_major );
        CHECK( FIntval("da3").get_type() == k_double_augmented );
        CHECK( FIntval("d4").get_type() == k_diminished );
        CHECK( FIntval("m6").get_type() == k_minor );
        CHECK( FIntval("p8").get_type() == k_perfect );
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_08)
    {
        //@08. negative intervals. Properties

        FIntval intv = FIntval("M2") - FIntval("M3");
        CHECK( int(intv) == -int(FIntval("M2")) );
        CHECK( intv.get_number() == 2 );
        CHECK( intv.get_type() == k_major );
        CHECK( intv.get_num_semitones() == 2 );
        CHECK( intv.get_code() == "M2" );
        CHECK( intv.is_descending() == true );
//        cout << "intv=" << int(intv) << ", number=" << intv.get_number()
//             << ", semitones=" << intv.get_num_semitones() << endl;
    }

    TEST_FIXTURE(IntervalTestFixture, fintval_09)
    {
        //@09. contructor from components. Negative intervals

        FIntval intv(-9, k_major);
        CHECK( int(intv) == -int(FIntval(9, k_major)) );
        CHECK( intv.get_number() == 9 );
        CHECK( intv.get_type() == k_major );
        CHECK( intv.get_num_semitones() == 14 );
        CHECK( intv.get_code() == "M9" );
        CHECK( intv.is_descending() == true );
    }


}


