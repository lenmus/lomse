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
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_lmd_exporter.h"
#include "lomse_ldp_compiler.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"
#include "lomse_xml_parser.h"
#include "lomse_lmd_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// test for LmdExporter
//=======================================================================================

//---------------------------------------------------------------------------------------
//helper, to replace lomse version and export time
class MyLmdExporter : public LmdExporter
{
public:
    MyLmdExporter(LibraryScope& scope, string lomseVersion, string exportTime)
        : LmdExporter(scope)
    {
        m_lomseVersion = lomseVersion;
        m_exportTime = exportTime;
    }
};

//---------------------------------------------------------------------------------------
class LmdExporterTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;

    LmdExporterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LmdExporterTestFixture()    //TearDown fixture
    {
    }

};

//---------------------------------------------------------------------------------------
SUITE(LmdExporterTest)
{
//    TEST_FIXTURE(LmdExporterTestFixture, visual)
//    {
//        //visual test to display the exported score
//
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "09002-ebook-example.lms" );
//        ImoDocument* pRoot = doc.get_imodoc();
//
//        LmdExporter exporter(m_libraryScope);
//        exporter.set_score_format(LmdExporter::k_format_ldp);
//        string source = exporter.get_source(pRoot);
//        cout << "----------------------------------------------------" << endl;
//        cout << source << endl;
//        cout << "----------------------------------------------------" << endl;
//    }

    // section (heading) ----------------------------------------------------------------

    TEST_FIXTURE(LmdExporterTestFixture, section)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<section level='1'>This is a header</section>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoObj* pImo = pIModel->get_root();

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<section level=\"1\">This is a header</section>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    TEST_FIXTURE(LmdExporterTestFixture, section_with_style)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Heading</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><section level='1' style='Heading'>Hello world!</section></content>"
            "</lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pImo = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<section level=\"1\" style=\"Heading\">Hello world!</section>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    TEST_FIXTURE(LmdExporterTestFixture, section_many_items_simplify)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<section level='1'>This is a header"
            "<txt> with two items.</txt></section>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoObj* pImo = pIModel->get_root();

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<section level=\"1\">This is a header with two items.</section>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    TEST_FIXTURE(LmdExporterTestFixture, section_many_items_with_styles)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Heading</name><color>#00fe0f7f</color></defineStyle>"
                    "<defineStyle><name>Other</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content>"
                    "<section level='1' style='Heading'>This is a header"
                    "<txt style='Other'> with two items.</txt> And more.</section>"
                "</content>"
            "</lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pImo = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<section level=\"1\" style=\"Heading\">This is a header"
            "<txt style=\"Other\"> with two items.</txt> And more.</section>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    // style ----------------------------------------------------------------------------

    TEST_FIXTURE(LmdExporterTestFixture, style)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        string src =
            "<defineStyle>"
                "<name>Header1</name>"
                "<font-name>Times New Roman</font-name>"
                "<font-size>14pt</font-size>"
                "<font-style>italic</font-style>"
                "<font-weight>bold</font-weight>"
                "<color>#00fe0f7f</color>"
            "</defineStyle>";
        parser.parse_text(src);
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoStyle* pImo = dynamic_cast<ImoStyle*>( pIModel->get_root() );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = src;
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    // styles ---------------------------------------------------------------------------

    TEST_FIXTURE(LmdExporterTestFixture, styles_empty)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        string src =
            "<styles>"
            "</styles>";
        parser.parse_text(src);
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoStyles* pImo = dynamic_cast<ImoStyles*>( pIModel->get_root() );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = src;
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

    TEST_FIXTURE(LmdExporterTestFixture, styles)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        string src =
            "<styles>"
                "<defineStyle>"
                    "<name>Header1</name>"
                    "<font-name>Times New Roman</font-name>"
                    "<font-size>14pt</font-size>"
                    "<font-style>italic</font-style>"
                    "<font-weight>bold</font-weight>"
                    "<color>#00fe0f7f</color>"
                "</defineStyle>"
            "</styles>";
        parser.parse_text(src);
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoStyles* pImo = dynamic_cast<ImoStyles*>( pIModel->get_root() );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = src;
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

//    TEST_FIXTURE(LmdExporterTestFixture, ErrorNotImplemented)
//    {
//        Document doc(m_libraryScope);
//        ImoTie* pTie = static_cast<ImoTie*>(ImFactory::inject(k_imo_tie, &doc));
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pTie);
////        cout << source << endl;
//        CHECK( source == "(TODO: tie   type=76, id=0 )\n" );
//        delete pTie;
//    }
//
//    // score ----------------------------------------------------------------------------
//
//    TEST_FIXTURE(LmdExporterTestFixture, score)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string(
//            "(lenmusdoc (vers 0.0) (content (score (vers 1.6)"
//            "(instrument (musicData (clef G)(key D)(n c4 q)(barline) ))"
//            ")))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pScore);
//        cout << "\"" << source << "\"" << endl;
//        CHECK( source ==
//              "(score (vers 1.6)\n"
//              "   (instrument \n"
//              "      (musicData \n"
//              "         (clef G p1)\n"
//              "         (key D)\n"
//              "         (n c4 q p1)\n"
//              "         (barline )\n"
//              ")\n)\n)" );
//    }
//
//    // clef ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LmdExporterTestFixture, clef)
//    {
//        Document doc(m_libraryScope);
//        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
//        pClef->set_clef_type(k_clef_F4);
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pClef);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef F4 p1)" );
//        delete pClef;
//    }
//
//    // note ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LmdExporterTestFixture, Note)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_D, k_octave_4,
//                            k_eighth, k_no_accidentals);
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pImo);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n d4 e p1)" );
//        delete pImo;
//    }
//
//    TEST_FIXTURE(LmdExporterTestFixture, Note_dots)
//    {
//        Document doc(m_libraryScope);
//        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_B, k_octave_7,
//                            k_whole, k_sharp, 2);
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pImo);
//        //cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(n +b7 w.. p1)" );
//        delete pImo;
//    }
//
//
//    // color ------------------------------------------------------------------------------------
//
//    TEST_FIXTURE(LmdExporterTestFixture, color)
//    {
//        Document doc(m_libraryScope);
//        ImoClef* pImo = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
//        pImo->set_clef_type(k_clef_F4);
//        pImo->set_color( Color(127, 40, 12, 128) );
//        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
//        string source = exporter.get_source(pImo);
////        cout << "\"" << source << "\"" << endl;
//        CHECK( source == "(clef F4 p1 (color #7f280c80))" );
//        delete pImo;
//    }
//
////    // user location ----------------------------------------------------------------------------
////
////    TEST_FIXTURE(LmdExporterTestFixture, ExportLdp_user_location)
////    {
////        ImoClef obj;
////        obj.set_type(k_clef_G2);
////        obj.set_user_location_x(30.0f);
////        obj.set_user_location_y(-7.05f);
////        LmdExporter exporter("0.12.5", "2012/12/21 13:10:27");
////        string source = exporter.get_source(&obj);
////        cout << "\"" << source << "\"" << endl;
////        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
////    }

    // lenmusdoc ------------------------------------------------------------------------

    TEST_FIXTURE(LmdExporterTestFixture, lenmusdoc)
    {
        Document doc(m_libraryScope);
        doc.from_string("<lenmusdoc vers='2.3'><content/></lenmusdoc>", Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_imodoc();

        MyLmdExporter exporter(m_libraryScope, "0.12.5", "2012/12/21 13:10:27");
        string source = exporter.get_source(pImoDoc);
        string expected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<lenmusdoc vers=\"2.3\" language=\"en\">\n"
            "   <!-- LMD file generated by Lomse, version 0.12.5, date 2012/12/21 13:10:27 -->\n"
            "   <styles>\n"
            "   </styles>\n\n"
            "   <content>\n"
            "   </content>\n"
            "</lenmusdoc>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );
    }

    TEST_FIXTURE(LmdExporterTestFixture, lenmusdoc_id)
    {
        Document doc(m_libraryScope);
        doc.from_string("<lenmusdoc vers='2.3'><content/></lenmusdoc>", Document::k_format_lmd);
        ImoDocument* pImoDoc = doc.get_imodoc();

        MyLmdExporter exporter(m_libraryScope, "0.12.5", "2012/12/21 13:10:27");
        exporter.set_add_id(true);
        string source = exporter.get_source(pImoDoc);
        string expected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<lenmusdoc id=\"0\" vers=\"2.3\" language=\"en\">\n"
            "   <!-- LMD file generated by Lomse, version 0.12.5, date 2012/12/21 13:10:27 -->\n"
            "   <styles id=\"1\">\n"
            "   </styles>\n\n"
            "   <content id=\"3\">\n"
            "   </content>\n"
            "</lenmusdoc>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );
    }

    TEST_FIXTURE(LmdExporterTestFixture, lenmusdoc_with_styles)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Heading</name><color>#00fe0f7f</color></defineStyle>"
                    "<defineStyle><name>Other</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content>"
                "</content>"
            "</lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pImo = dynamic_cast<ImoDocument*>( pIModel->get_root() );

        LmdExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<lenmusdoc vers=\"0.0\" language=\"en\">"
                "<styles>"
                    "<defineStyle><name>Heading</name><color>#00fe0f7f</color></defineStyle>"
                    "<defineStyle><name>Other</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content>"
                "</content>"
            "</lenmusdoc>";
//        cout << source << endl;
//        cout << expected << endl;
        CHECK( source == expected );

        delete pIModel;
    }

};
