//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
#include "lomse_injectors.h"
#include "lomse_lmd_parser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LmdParserTestFixture
{
public:

    LmdParserTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~LmdParserTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(LmdParserTest)
{
    TEST_FIXTURE(LmdParserTestFixture, root_node_no_xml)
    {
        LmdParser parser;
        parser.parse_text("<score><vers>1.7</vers></score>");
        XmlNode* root = parser.get_tree_root();
        CHECK( parser.get_node_name_as_string(root) == "score" );
        CHECK( parser.get_encoding() == "unknown" );
    }

    TEST_FIXTURE(LmdParserTestFixture, root_node_xml)
    {
        LmdParser parser;
        string text(
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
            "<lenmusdoc"
            "    xmlns=\"http://www.lenmus.org\""
            "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
            "    xsi:schemaLocation=\"http://www.lenmus.org lenmusdoc.xsd\""
            "    vers=\"0.0\">"
            ""
            "<score>"
                "<vers>1.7</vers>"
            "</score>"
            "</lenmusdoc>"
        );
        parser.parse_text(text);
        XmlNode* root = parser.get_tree_root();
        CHECK( parser.get_node_name_as_string(root) == "lenmusdoc" );
        CHECK( parser.get_encoding() == "utf-8" );
    }

    TEST_FIXTURE(LmdParserTestFixture, invalid_xml)
    {
        LmdParser parser;
        string text(
            "<lenmusdoc vers=\"0.0\">"
            "<score>"
                "<vers>1.7</vers>"
            "</score>"
        );
        parser.parse_text(text);
        XmlNode* root = parser.get_tree_root();
        CHECK( root == NULL );
        //cout << "error = [" << parser.get_error() << "]" << endl;
        CHECK( parser.get_error() == "unexpected end of data" );
    }

    TEST_FIXTURE(LmdParserTestFixture, read_doc_from_file)
    {
        LmdParser parser;
        parser.parse_file(m_scores_path + "30001-paragraph.lmd");
        XmlNode* root = parser.get_tree_root();

        CHECK( parser.get_node_name_as_string(root) == "lenmusdoc" );
    }

    //TEST_FIXTURE(LmdParserTestFixture, ParserFileHasLineNumbers)
    //{
    //    LmdParser parser;

    //    parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
    //    XmlNode* root = parser.get_tree_root();
    //    LdpElement* elm = score->get_root();
    //    //cout << elm->get_name() << " in line " << elm->get_line_number() << endl;
    //    CHECK( elm->get_name() == "score" && elm->get_line_number() == 1 );
    //    elm = elm->get_first_child();
    //    CHECK( elm->get_name() == "vers" && elm->get_line_number() == 2 );
    //    elm = elm->get_next_sibling();
    //    CHECK( elm->get_name() == "systemLayout" && elm->get_line_number() == 3 );
    //    elm = elm->get_next_sibling();
    //    CHECK( elm->get_name() == "systemLayout" && elm->get_line_number() == 4 );
    //    elm = elm->get_next_sibling();
    //    CHECK( elm->get_name() == "opt" && elm->get_line_number() == 5 );
    //    elm = elm->get_next_sibling();
    //    CHECK( elm->get_name() == "opt" && elm->get_line_number() == 6 );
    //    elm = elm->get_next_sibling();
    //    CHECK( elm->get_name() == "instrument" && elm->get_line_number() == 7 );

    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserReadScoreFromUnicodeFile)
    //{
    //    LmdParser parser;
    //    parser.parse_file(m_scores_path + "00002-unicode-text.lms");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(score (vers 1.6) (language en iso-8859-1) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (clef G) (n c4 q) (text \"Текст на кирилица\" (dx 15) (dy -10) (font normal 10)))))" );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserElementWithId)
    //{
    //    LmdParser parser;
    //    parser.parse_text("(clef#27 G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 27L );
    //    CHECK( parser.get_max_id() == 27L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserElementWithoutId)
    //{
    //    LmdParser parser;
    //    parser.parse_text("(clef G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    CHECK( parser.get_max_id() == 0L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserIdsInSequence)
    //{
    //    LmdParser parser;
    //    parser.parse_file(m_scores_path + "00011-empty-fill-page.lms");
    //    XmlNode* root = parser.get_tree_root();
    //    LdpElement* elm = score->get_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    //cout << elm->get_name() << ". Id = " << elm->get_id() << endl;
    //    CHECK( elm->get_id() == 0L );
    //    elm = elm->get_first_child();   //vers
    //    CHECK( elm->get_id() == 1L );
    //    elm = elm->get_next_sibling();  //systemLayout + systemMargins
    //    CHECK( elm->get_id() == 2L );
    //    elm = elm->get_next_sibling();  //systemLayout + systemMargins
    //    CHECK( elm->get_id() == 4L );
    //    elm = elm->get_next_sibling();  //opt
    //    CHECK( elm->get_id() == 6L );
    //    elm = elm->get_next_sibling();  //opt
    //    CHECK( elm->get_id() == 7L );
    //    elm = elm->get_next_sibling();  //instrument
    //    CHECK( elm->get_id() == 8L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserElementWithBadId)
    //{
    //    stringstream errormsg;
    //    LmdParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Bad id in name 'clef#three'." << endl;
    //    parser.parse_text("(clef#three G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << errormsg.str();
    //    //cout << expected.str();
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    CHECK( parser.get_max_id() == 0L );
    //    CHECK( errormsg.str() == expected.str() );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserMinusSign)
    //{
    //    stringstream errormsg;
    //    LmdParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Bad id in name 'clef#three'." << endl;
    //    parser.parse_text("(t -)");
    //    XmlNode* root = parser.get_tree_root();
    //    cout << errormsg.str();
    //    cout << expected.str();
    //    CHECK( score->get_root()->to_string() == "(t -)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    CHECK( parser.get_max_id() == 0L );
    //    CHECK( errormsg.str() == expected.str() );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserElementWithLowId)
    //{
    //    stringstream errormsg;
    //    LmdParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. In 'key#1'. Value for id already exists. Ignored." << endl;
    //    parser.parse_text("(musicData (clef G) (key#1 D))");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << errormsg.str();
    //    //cout << expected.str();
    //    LdpElement* elm = score->get_root();
    //    CHECK( elm->to_string() == "(musicData (clef G) (key D))" );
    //    CHECK( elm->get_id() == 0L );    //musicData
    //    elm = elm->get_first_child();   //clef
    //    CHECK( elm->to_string() == "(clef G)" );
    //    CHECK( elm->get_id() == 1L );
    //    elm = elm->get_next_sibling();  //key
    //    CHECK( elm->to_string() == "(key D)" );
    //    CHECK( elm->get_id() == 2L );
    //    CHECK( errormsg.str() == expected.str() );
    //    CHECK( parser.get_max_id() == 2L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserElementWithHighId)
    //{
    //    stringstream errormsg;
    //    LmdParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    parser.parse_text("(musicData (clef#3 G) (key D))");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << errormsg.str();
    //    //cout << expected.str();
    //    LdpElement* elm = score->get_root();
    //    CHECK( elm->get_id() == 0L );    //musicData
    //    elm = elm->get_first_child();   //clef
    //    CHECK( elm->get_id() == 3L );
    //    elm = elm->get_next_sibling();  //key
    //    CHECK( elm->get_id() == 4L );
    //    CHECK( errormsg.str() == expected.str() );
    //    CHECK( parser.get_max_id() == 4L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserReplaceNoVisible)
    //{
    //    LmdParser parser;
    //    parser.parse_text("(clef G noVisible (dx 70))");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx 70))" );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserReplaceNoVisibleNothingAfter)
    //{
    //    LmdParser parser;
    //    parser.parse_text("(clef G noVisible)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G (visible no))" );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(LmdParserTestFixture, ParserReadChinese)
    //{
    //    LmdParser parser;
    //    parser.parse_text("(heading 1 (style \"header\")(txt \"普通练习\"))");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(heading 1 (style \"header\") (txt \"普通练习\"))" );
    //    delete score->get_root();
    //}

};

