//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_tokenizer.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpTokenizerTestFixture
{
public:

    LdpTokenizerTestFixture()     //SetUp fixture
    {
        m_scores_path = TESTLIB_SCORES_PATH;
    }

    ~LdpTokenizerTestFixture()    //TearDown fixture
    {
    }

    std::string m_scores_path;
};

SUITE(LdpTokenizerTest)
{
    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadTokens)
    {
        LdpTextReader reader("(score blue)");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token != NULL );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadRightTokens)
    {
        LdpTextReader reader("(score blue)");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkStartOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "score" );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "blue" );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkEndOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkEndOfFile );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerCanReadFile)
    {
        LdpFileReader reader(m_scores_path + "00011-empty-fill-page.lms");
        LdpTokenizer tokenizer(reader, cout);
        int numTokens = 0;
        for (LdpToken* token = tokenizer.read_token();
             token->get_type() != tkEndOfFile;
             token = tokenizer.read_token())
        {
            numTokens++;
            //cout << token->get_value() << endl;
        }
        CHECK( reader.end_of_data() );
        CHECK( numTokens == 45 );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerCanReadUnicodeString)
    {
        //cout << "'" << "Текст на кирилица" << "'" << endl;
        LdpFileReader reader(m_scores_path + "10021-unicode-text.lms");
        LdpTokenizer tokenizer(reader, cout);
        int numTokens = 0;
        LdpToken* token = tokenizer.read_token();
        for (; token->get_type() != tkEndOfFile; token = tokenizer.read_token())
        {
            numTokens++;
            if (token->get_type() == tkString)
                break;
        }
        //cout << "'" << token->get_value() << "'" << endl;
        //cout << "num.tokens=" << numTokens << endl;
        CHECK( token->get_type() == tkString );
        CHECK( numTokens == 22 );
        CHECK( token->get_value() == "音乐老师  Текст на кирилица" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadIntegerNumber)
    {
        LdpTextReader reader("(dx 15)");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkStartOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "dx" );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkIntegerNumber );
        CHECK( token->get_value() == "15" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadCompactNotation)
    {
        LdpTextReader reader("dx:15");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkStartOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "dx" );
        token = tokenizer.read_token();
        //cout << "type='" << token->get_type() << "' value='"
        //      << "' value='" << token->get_value() << "'" << endl;
        CHECK( token->get_type() == tkIntegerNumber );
        CHECK( token->get_value() == "15" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadCompactNotationTwo)
    {
        LdpTextReader reader("dx:15 dy:12.77");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkStartOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "dx" );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkIntegerNumber );
        CHECK( token->get_value() == "15" );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkEndOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkStartOfElement );
        token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "dy" );
        token = tokenizer.read_token();
        //cout << "type='" << token->get_type() << "' value='"
        //      << "' value='" << token->get_value() << "'" << endl;
        CHECK( token->get_type() == tkRealNumber );
        CHECK( token->get_value() == "12.77" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadStringSimpleQuotes)
    {
        LdpTextReader reader(" ''this is a string'' ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkString );
        CHECK( token->get_value() == "this is a string" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadStringDoubleQuotes)
    {
        LdpTextReader reader(" \"this is a string\" ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkString );
        CHECK( token->get_value() == "this is a string" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadStringSimpleQuotesTranslatable)
    {
        LdpTextReader reader(" _''this is a string'' ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkString );
        CHECK( token->get_value() == "this is a string" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadStringDoubleQuotesTranslatable)
    {
        LdpTextReader reader(" _\"this is a string\" ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkString );
        CHECK( token->get_value() == "this is a string" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadLabel_1)
    {
        LdpTextReader reader(" #00ff45 ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "#00ff45" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadLabel_2)
    {
        LdpTextReader reader(" score ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "score" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadLabel_3)
    {
        LdpTextReader reader(" 45a ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "45a" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerReadLabel_4)
    {
        LdpTextReader reader(" - ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkLabel );
        CHECK( token->get_value() == "-" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerPositiveIntegerNumber)
    {
        LdpTextReader reader(" +45 ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkIntegerNumber );
        CHECK( token->get_value() == "+45" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerNegativeIntegerNumber)
    {
        LdpTextReader reader(" -45 ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkIntegerNumber );
        CHECK( token->get_value() == "-45" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerPositiveRealNumber)
    {
        LdpTextReader reader(" +45.98 ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkRealNumber );
        CHECK( token->get_value() == "+45.98" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, TokenizerNegativeRealNumber)
    {
        LdpTextReader reader(" -45.70 ");
        LdpTokenizer tokenizer(reader, cout);
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkRealNumber );
        CHECK( token->get_value() == "-45.70" );
    }

    TEST_FIXTURE(LdpTokenizerTestFixture, Tokenizer_skip_bom)
    {
        LdpTextReader reader("\xef\xbb\xbf -45.70 ");
        LdpTokenizer tokenizer(reader, cout);
        tokenizer.skip_utf_bom();
        LdpToken* token = tokenizer.read_token();
        CHECK( token->get_type() == tkRealNumber );
        CHECK( token->get_value() == "-45.70" );
    }

};
