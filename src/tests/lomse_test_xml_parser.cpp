//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2014 Cecilio Salmeron. All rights reserved.
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
#include "lomse_xml_parser.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class XmlParserTestFixture
{
public:

    XmlParserTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = LOMSE_NEW LibraryScope(cout);
        m_pLibraryScope->set_default_fonts_path(TESTLIB_FONTS_PATH);
        m_scores_path = TESTLIB_SCORES_PATH;
    }

    ~XmlParserTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(XmlParserTest)
{
    TEST_FIXTURE(XmlParserTestFixture, root_node_no_xml)
    {
        XmlParser parser;
        parser.parse_text("<score><vers>1.7</vers></score>");
        XmlNode* root = parser.get_tree_root();
        CHECK( root->name() == "score" );
        CHECK( parser.get_encoding() == "unknown" );
    }

    TEST_FIXTURE(XmlParserTestFixture, root_node_xml)
    {
        XmlParser parser;
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
        CHECK( root->name() == "lenmusdoc" );
        //cout << "encoding: " << parser.get_encoding() << endl;
        CHECK( parser.get_encoding() == "utf-8" );
    }

    TEST_FIXTURE(XmlParserTestFixture, invalid_xml)
    {
        XmlParser parser;
        string text(
            "<lenmusdoc vers=\"0.0\">"
            "<score>"
                "<vers>1.7</vers>"
            "</score>"
        );
        parser.parse_text(text);
        //cout << "error = [" << parser.get_error() << "]" << endl;
        CHECK( parser.get_error() == "Start-end tags mismatch" );
    }

    TEST_FIXTURE(XmlParserTestFixture, read_doc_from_file)
    {
        XmlParser parser;
        parser.parse_file(m_scores_path + "30001-paragraph.lmd");
        XmlNode* root = parser.get_tree_root();

        CHECK( root->name() == "lenmusdoc" );
    }

    TEST_FIXTURE(XmlParserTestFixture, read_chinese)
    {
        XmlParser parser;
        string text(
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
            "<lenmusdoc vers=\"0.0\">"
            "<header>"
                "<txt>普通练习</txt>"
            "</header>"
            "</lenmusdoc>"
        );
        parser.parse_text(text);
        XmlNode* root = parser.get_tree_root();
        CHECK( root->name() == "lenmusdoc" );
        CHECK( parser.get_encoding() == "utf-8" );
        XmlNode header = root->child("header");
        XmlNode txt = header.child("txt");
        //cout << "Node txt has value: '" << header.value() << "'" << endl;
        CHECK( txt.value() == "普通练习" );

    }

    TEST_FIXTURE(XmlParserTestFixture, empty_element)
    {
        XmlParser parser;
        parser.parse_text("<score-partwise version='3.0'><part-list/></score-partwise>");
        XmlNode* root = parser.get_tree_root();
        CHECK( root->name() == "score-partwise" );
        XmlNode child = root->first_child();
        cout << "Child name: '" << child.value() << "'" << endl;
        CHECK( child.name() == "part-list" );

    }



    //TEST_FIXTURE(XmlParserTestFixture, ParserReadScoreFromUnicodeFile)
    //{
    //    XmlParser parser;
    //    parser.parse_file(m_scores_path + "00002-unicode-text.lms");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(score (vers 1.6) (language en iso-8859-1) (systemLayout first (systemMargins 0 0 0 2000)) (systemLayout other (systemMargins 0 0 1200 2000)) (opt Score.FillPageWithEmptyStaves true) (opt StaffLines.StopAtFinalBarline false) (instrument (musicData (clef G) (n c4 q) (text \"Текст на кирилица\" (dx 15) (dy -10) (font normal 10)))))" );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserElementWithId)
    //{
    //    XmlParser parser;
    //    parser.parse_text("(clef#27 G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 27L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserElementWithoutId)
    //{
    //    XmlParser parser;
    //    parser.parse_text("(clef G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserIdsInSequence)
    //{
    //    XmlParser parser;
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

    //TEST_FIXTURE(XmlParserTestFixture, ParserElementWithBadId)
    //{
    //    stringstream errormsg;
    //    XmlParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Bad id in name 'clef#three'." << endl;
    //    parser.parse_text("(clef#three G)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << errormsg.str();
    //    //cout << expected.str();
    //    CHECK( score->get_root()->to_string() == "(clef G)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    CHECK( errormsg.str() == expected.str() );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserMinusSign)
    //{
    //    stringstream errormsg;
    //    XmlParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Bad id in name 'clef#three'." << endl;
    //    parser.parse_text("(t -)");
    //    XmlNode* root = parser.get_tree_root();
    //    cout << errormsg.str();
    //    cout << expected.str();
    //    CHECK( score->get_root()->to_string() == "(t -)" );
    //    CHECK( score->get_root()->get_id() == 0L );
    //    CHECK( errormsg.str() == expected.str() );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserElementWithHighId)
    //{
    //    stringstream errormsg;
    //    XmlParser parser(errormsg, m_pLibraryScope->ldp_factory());
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
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserReplaceNoVisible)
    //{
    //    XmlParser parser;
    //    parser.parse_text("(clef G noVisible (dx 70))");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G (visible no) (dx 70))" );
    //    delete score->get_root();
    //}

    //TEST_FIXTURE(XmlParserTestFixture, ParserReplaceNoVisibleNothingAfter)
    //{
    //    XmlParser parser;
    //    parser.parse_text("(clef G noVisible)");
    //    XmlNode* root = parser.get_tree_root();
    //    //cout << score->get_root()->to_string() << endl;
    //    CHECK( score->get_root()->to_string() == "(clef G (visible no))" );
    //    delete score->get_root();
    //}

};

