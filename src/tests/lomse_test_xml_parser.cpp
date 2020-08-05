//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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
#include "private/lomse_document_p.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class XmlParserTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    XmlParserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        m_scores_path = TESTLIB_SCORES_PATH;
    }

    ~XmlParserTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

SUITE(XmlParserTest)
{
    TEST_FIXTURE(XmlParserTestFixture, xml_parser_01)
    {
        //@01. root node. No <?xml> declaration tag

        XmlParser parser;
        parser.parse_text("<score><vers>1.7</vers></score>");
        XmlNode* root = parser.get_tree_root();
        CHECK( root->name() == "score" );
        CHECK( parser.get_encoding() == "unknown" );
    }

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_02)
    {
        //@02. root node. <?xml> tag exist. Encoding extracted

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

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_03)
    {
        //@03. invalid xml. tags mismatch

        stringstream errormsg;
        stringstream expected;
        expected << "Pos: 52. Error: Start-end tags mismatch" << endl;
        XmlParser parser(errormsg);
        string text(
            "<lenmusdoc vers=\"0.0\">"
            "<score>"
                "<vers>1.7</vers>"
            "</score>"
        );
        parser.parse_text(text);
        //cout << "error = [" << parser.get_error() << "]" << endl;
        CHECK( parser.get_error() == "Start-end tags mismatch" );

        CHECK( errormsg.str() == expected.str() );
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
    }

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_04)
    {
        //@04. Read content from file

        XmlParser parser;
        parser.parse_file(m_scores_path + "08011-paragraph.lmd");
        XmlNode* root = parser.get_tree_root();

        CHECK( root->name() == "lenmusdoc" );
    }

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_05)
    {
        //@05. Read chinese texts

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

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_06)
    {
        //@06. Empty element

        XmlParser parser;
        parser.parse_text("<score-partwise version='3.0'><part-list/></score-partwise>");
        XmlNode* root = parser.get_tree_root();
        CHECK( root->name() == "score-partwise" );
        XmlNode child = root->first_child();
        //cout << "Child name: '" << child.value() << "'" << endl;
        CHECK( child.name() == "part-list" );

    }

    TEST_FIXTURE(XmlParserTestFixture, xml_parser_901)
    {
        //@901. File not found

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        string filename = "non-existing-path/to/nowhere/no-file.xml";
        stringstream expected;
        expected << "Pos: 0. Error: File was not found" << ". File=" << filename << endl;
        parser.parse_file(filename);

        CHECK( parser.get_error() == "File was not found" );
        //cout << test_name() << ", error=" << parser.get_error() << endl;

        XmlNode* tree = parser.get_tree_root();

        CHECK( errormsg.str() == expected.str() );
//        cout << test_name() << endl;
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;

        CHECK( tree != nullptr);
    }


};

