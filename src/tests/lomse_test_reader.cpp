//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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
#include <iostream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_reader.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;

class LdpTextReaderTestFixture
{
public:

    LdpTextReaderTestFixture()     //SetUp fixture
    {
    }

    ~LdpTextReaderTestFixture()    //TearDown fixture
    {
    }
};

SUITE(LdpTextReaderTest)
{
    TEST_FIXTURE(LdpTextReaderTestFixture, CanCreateTextReader)
    {
        LdpTextReader reader("123 abc");
        CHECK( reader.is_ready() );
    }

    TEST_FIXTURE(LdpTextReaderTestFixture, CanReadFromTextReader)
    {
        LdpTextReader reader("123 abc");
        CHECK( reader.get_next_char() == '1' );
        CHECK( reader.get_next_char() == '2' );
        CHECK( reader.get_next_char() == '3' );
        CHECK( reader.get_next_char() == ' ' );
        CHECK( reader.get_next_char() == 'a' );
        CHECK( reader.get_next_char() == 'b' );
        CHECK( reader.get_next_char() == 'c' );
    }

    TEST_FIXTURE(LdpTextReaderTestFixture, TextReaderReturnsEOF)
    {
        LdpTextReader reader("abc");
        CHECK( reader.get_next_char() == 'a' );
        CHECK( reader.get_next_char() == 'b' );
        CHECK( reader.get_next_char() == 'c' );
        CHECK( reader.get_next_char() == EOF );
    }

    TEST_FIXTURE(LdpTextReaderTestFixture, TextReaderEndOfDataWorks)
    {
        LdpTextReader reader("abc");
        CHECK( reader.get_next_char() == 'a' );
        CHECK( reader.get_next_char() == 'b' );
        CHECK( reader.get_next_char() == 'c' );
        CHECK( reader.end_of_data() );
    }

    TEST_FIXTURE(LdpTextReaderTestFixture, TextReaderCanUnreadOneChar)
    {
        LdpTextReader reader("abc");
        reader.get_next_char();
        CHECK( reader.get_next_char() == 'b' );
        reader.repeat_last_char();
        CHECK( reader.get_next_char() == 'b' );
        CHECK( reader.get_next_char() == 'c' );
        CHECK( reader.end_of_data() );
    }

    TEST_FIXTURE(LdpTextReaderTestFixture, TextReaderKnowsItsLocator)
    {
        LdpTextReader reader("abc");
        CHECK( reader.get_locator() == "string:");
    }

}

//---------------------------------------------------------------------------
class LdpFileReaderTestFixture
{
public:

    LdpFileReaderTestFixture()     //SetUp fixture
    {
        m_scores_path = TESTLIB_SCORES_PATH;
    }

    ~LdpFileReaderTestFixture()    //TearDown fixture
    {
    }

    std::string m_scores_path;
};

SUITE(LdpFileReaderTest)
{
    TEST_FIXTURE(LdpFileReaderTestFixture, InvalidFileThrowException)
    {
        bool fOk = false;
        try
        {
            LdpFileReader reader("../../invalid_path/no-score.lms");
        }
        catch(exception& e)
        {
            //cout << e.what() << endl;
            e.what();
            fOk = true;
        }
        CHECK(fOk);
    }

    TEST_FIXTURE(LdpFileReaderTestFixture, FileReaderCanOpenFile)
    {
        try
        {
            LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
            CHECK( reader.is_ready() );
        }
        catch(const exception& e)
        {
            cout << e.what() << endl;
            CHECK( false );
        }
    }

    TEST_FIXTURE(LdpFileReaderTestFixture, CanReadFromFileReader)
    {
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        CHECK( reader.get_next_char() == '(' );
    }

    TEST_FIXTURE(LdpFileReaderTestFixture, FileReaderCanUnreadOneChar)
    {
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        reader.get_next_char();
        CHECK( reader.get_next_char() == 's' );
        reader.repeat_last_char();
        CHECK( reader.get_next_char() == 's' );
        CHECK( reader.get_next_char() == 'c' );
    }

    TEST_FIXTURE(LdpFileReaderTestFixture, FileReaderReachesEndOfData)
    {
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        CHECK( reader.end_of_data() == false );
        CHECK( reader.get_next_char() != EOF );
        while( !reader.end_of_data())
            reader.get_next_char();
        CHECK( reader.end_of_data() );
    }

    TEST_FIXTURE(LdpFileReaderTestFixture, FileReaderKnowsItsLocator)
    {
        string loc = m_scores_path + "00011-empty-fill-page.lms";
        LdpFileReader reader(loc);
        CHECK( reader.get_locator() == loc);
    }

}
