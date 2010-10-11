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

#include <UnitTest++.h>
#include <iostream>
#include "lomse_config.h"

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

}

//---------------------------------------------------------------------------
class LdpFileReaderTestFixture
{
public:

    LdpFileReaderTestFixture()     //SetUp fixture
    {
        m_scores_path = LOMSE_TEST_SCORES_PATH;
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

}
