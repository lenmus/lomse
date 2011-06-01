//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <iostream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_parser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpParserTestFixture
{
public:

    LdpParserTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~LdpParserTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(LdpParserTest)
{
    TEST_FIXTURE(LdpParserTestFixture, ParserReadTokensFromText)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_text("(score (vers 1.7))");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(score (vers 1.7))" );
            //"(score (vers 1.6) (language en utf-8) (instrument (musicData )))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReadScoreFromFile)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
        //cout << score->get_root()->to_string() << endl;

        CHECK( score->get_root()->to_string() ==
            "(score (vers 1.6) (systemLayout first (systemMargins 0 0 1500 1500)) "
            "(systemLayout other (systemMargins 0 0 1500 1000)) "
            "(opt Score.FillPageWithEmptyStaves true) "
            "(opt StaffLines.StopAtFinalBarline false) "
            "(instrument (musicData )))" );

        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserFileHasLineNumbers)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());

        SpLdpTree score = parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
        LdpElement* elm = score->get_root();
        //cout << elm->get_name() << " in line " << elm->get_line_number() << endl;
        CHECK( elm->get_name() == "score" && elm->get_line_number() == 1 );
        elm = elm->get_first_child();
        CHECK( elm->get_name() == "vers" && elm->get_line_number() == 2 );
        elm = elm->get_next_sibling();
        CHECK( elm->get_name() == "systemLayout" && elm->get_line_number() == 3 );
        elm = elm->get_next_sibling();
        CHECK( elm->get_name() == "systemLayout" && elm->get_line_number() == 4 );
        elm = elm->get_next_sibling();
        CHECK( elm->get_name() == "opt" && elm->get_line_number() == 5 );
        elm = elm->get_next_sibling();
        CHECK( elm->get_name() == "opt" && elm->get_line_number() == 6 );
        elm = elm->get_next_sibling();
        CHECK( elm->get_name() == "instrument" && elm->get_line_number() == 7 );

        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReadScoreFromUnicodeFile)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_file(m_scores_path + "00002-unicode-text.lms");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(score (vers 1.6) (language en iso-8859-1) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (clef G) (n c4 q) (text \"Текст на кирилица\" (dx 15) (dy -10) (font normal 10)))))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithId)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_text("(clef#27 G)");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == 27L );
        CHECK( parser.get_max_id() == 27L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithoutId)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_text("(clef G)");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == 0L );
        CHECK( parser.get_max_id() == 0L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserIdsInSequence)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
        LdpElement* elm = score->get_root();
        //cout << score->get_root()->to_string() << endl;
        //cout << elm->get_name() << ". Id = " << elm->get_id() << endl;
        CHECK( elm->get_id() == 0L );
        elm = elm->get_first_child();   //vers
        CHECK( elm->get_id() == 1L );
        elm = elm->get_next_sibling();  //systemLayout + systemMargins
        CHECK( elm->get_id() == 2L );
        elm = elm->get_next_sibling();  //systemLayout + systemMargins
        CHECK( elm->get_id() == 4L );
        elm = elm->get_next_sibling();  //opt
        CHECK( elm->get_id() == 6L );
        elm = elm->get_next_sibling();  //opt
        CHECK( elm->get_id() == 7L );
        elm = elm->get_next_sibling();  //instrument
        CHECK( elm->get_id() == 8L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithBadId)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "Line 0. Bad id in name 'clef#three'." << endl;
        SpLdpTree score = parser.parse_text("(clef#three G)");
        //cout << errormsg.str();
        //cout << expected.str();
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == 0L );
        CHECK( parser.get_max_id() == 0L );
        CHECK( errormsg.str() == expected.str() );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserMinusSign)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        //expected << "Line 0. Bad id in name 'clef#three'." << endl;
        SpLdpTree score = parser.parse_text("(t -)");
        cout << errormsg.str();
        cout << expected.str();
        CHECK( score->get_root()->to_string() == "(t -)" );
        CHECK( score->get_root()->get_id() == 0L );
        CHECK( parser.get_max_id() == 0L );
        CHECK( errormsg.str() == expected.str() );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithLowId)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "Line 0. In 'key#1'. Value for id already exists. Ignored." << endl;
        SpLdpTree score = parser.parse_text("(musicData (clef G) (key#1 D))");
        //cout << errormsg.str();
        //cout << expected.str();
        LdpElement* elm = score->get_root();
        CHECK( elm->to_string() == "(musicData (clef G) (key D))" );
        CHECK( elm->get_id() == 0L );    //musicData
        elm = elm->get_first_child();   //clef
        CHECK( elm->to_string() == "(clef G)" );
        CHECK( elm->get_id() == 1L );
        elm = elm->get_next_sibling();  //key
        CHECK( elm->to_string() == "(key D)" );
        CHECK( elm->get_id() == 2L );
        CHECK( errormsg.str() == expected.str() );
        CHECK( parser.get_max_id() == 2L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithHighId)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        SpLdpTree score = parser.parse_text("(musicData (clef#3 G) (key D))");
        //cout << errormsg.str();
        //cout << expected.str();
        LdpElement* elm = score->get_root();
        CHECK( elm->get_id() == 0L );    //musicData
        elm = elm->get_first_child();   //clef
        CHECK( elm->get_id() == 3L );
        elm = elm->get_next_sibling();  //key
        CHECK( elm->get_id() == 4L );
        CHECK( errormsg.str() == expected.str() );
        CHECK( parser.get_max_id() == 4L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReplaceNoVisible)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_text("(clef G noVisible (dx 70))");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx 70))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReplaceNoVisibleNothingAfter)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        SpLdpTree score = parser.parse_text("(clef G noVisible)");
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no))" );
        delete score->get_root();
    }

};

