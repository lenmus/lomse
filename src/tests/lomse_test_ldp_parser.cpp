//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <iostream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_parser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LdpParserTestFixture
{
public:

    LdpParserTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
        m_scores_path = TESTLIB_SCORES_PATH;
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
        parser.parse_text("(score (vers 1.7))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(score (vers 1.7))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReadScoreFromFile)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;

        //AWARE: As analysis is not done, opt StopAtFinalBarline is not replaced
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

        parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
        LdpTree* score = parser.get_ldp_tree();
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
        parser.parse_file(m_scores_path + "10021-unicode-text.lms");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() ==
              "(score (vers 1.6) (instrument (musicData (clef G) "
              "(n c4 q) (text \"音乐老师  Текст на кирилица\" (dx 15) "
              "(dy -10)))))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithId)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_text("(clef#27 G)");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == 27L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithoutId)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_text("(clef G)");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == -1L );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithBadId)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "Line 0. Bad id in name 'clef#three'." << endl;
        parser.parse_text("(clef#three G)");
        LdpTree* score = parser.get_ldp_tree();
        //cout << errormsg.str();
        //cout << expected.str();
        CHECK( score->get_root()->to_string() == "(clef G)" );
        CHECK( score->get_root()->get_id() == -1L );
        CHECK( errormsg.str() == expected.str() );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserMinusSign)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        //expected << "Line 0. Bad id in name 'clef#three'." << endl;
        parser.parse_text("(t -)");
        LdpTree* score = parser.get_ldp_tree();
        cout << errormsg.str();
        cout << expected.str();
        CHECK( score->get_root()->to_string() == "(t -)" );
        CHECK( score->get_root()->get_id() == -1L );
        CHECK( errormsg.str() == expected.str() );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserElementWithHighId)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("(musicData (clef#3 G) (key D))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << errormsg.str();
        //cout << expected.str();
        LdpElement* elm = score->get_root();
        CHECK( elm->get_id() == -1L );    //musicData
        elm = elm->get_first_child();   //clef
        CHECK( elm->get_id() == 3L );
        elm = elm->get_next_sibling();  //key
        CHECK( elm->get_id() == -1L );
        CHECK( errormsg.str() == expected.str() );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReplaceNoVisible)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_text("(clef G noVisible (dx 70))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx 70))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReplaceNoVisibleNothingAfter)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_text("(clef G noVisible)");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserReadChinese)
    {
        LdpParser parser(cout, m_pLibraryScope->ldp_factory());
        parser.parse_text("(heading 1 (style \"header\")(txt \"普通练习\"))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(heading 1 (style \"header\") (txt \"普通练习\"))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserUnclosedComment_1)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "** LDP ERROR **: Syntax error. State 2, TkType 6, tkValue <>" << endl;
        parser.parse_text("(clef G noVisible (dx //comment 70))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << errormsg.str();
        //cout << expected.str();
        //cout << score->get_root()->to_string() << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx ))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserUnclosedComment_2)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "** LDP ERROR **: Syntax error. State 2, TkType 6, tkValue <>" << endl;
        parser.parse_text("(clef G noVisible (dx /*comment 70))*");
        LdpTree* score = parser.get_ldp_tree();
        //cout << errormsg.str();
        //cout << expected.str();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx ))" );
        delete score->get_root();
    }

    TEST_FIXTURE(LdpParserTestFixture, ParserUnclosedComment_3)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        expected << "** LDP ERROR **: Syntax error. State 2, TkType 6, tkValue <>" << endl;
        parser.parse_text("(clef G noVisible (dx /*comment 70))");
        LdpTree* score = parser.get_ldp_tree();
        //cout << errormsg.str();
        //cout << expected.str();
        //cout << score->get_root()->to_string() << endl;
        CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx ))" );
        delete score->get_root();
    }

};

