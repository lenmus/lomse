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

#define LOMSE_INTERNAL_API
#include <UnitTest++.h>
#include <iostream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_xml_parser.h"
#include "lomse_lmd_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
//#include "lomse_im_figured_bass.h"
//#include "lomse_pitch.h"
#include "lomse_events.h"
#include "lomse_doorway.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// access to protected members
class MyLmdAnalyser : public LmdAnalyser
{
public:
    MyLmdAnalyser(ostream& reporter, LibraryScope& libScope, Document* pDoc,
                  XmlParser* parser)
        : LmdAnalyser(reporter, libScope, pDoc, parser)
    {
    }

};

//---------------------------------------------------------------------------------------
class LmdAnalyserTestFixture
{
public:
    LibraryScope m_libraryScope;
    int m_requestType;
    bool m_fRequestReceived;
    ImoDocument* m_pDoc;


    LmdAnalyserTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_requestType(k_null_request)
        , m_fRequestReceived(false)
        , m_pDoc(NULL)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~LmdAnalyserTestFixture()    //TearDown fixture
    {
    }

    static void wrapper_lomse_request(void* pThis, Request* pRequest)
    {
        static_cast<LmdAnalyserTestFixture*>(pThis)->on_lomse_request(pRequest);
    }

    void on_lomse_request(Request* pRequest)
    {
        m_fRequestReceived = true;
        m_requestType = pRequest->get_request_type();
        if (m_requestType == k_dynamic_content_request)
        {
            RequestDynamic* pRq = dynamic_cast<RequestDynamic*>(pRequest);
            ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pRq->get_object() );
            m_pDoc = pDyn->get_document();
        }
    }

};


SUITE(LmdAnalyserTest)
{
    //@ preliminary-tests ---------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, analyse_node)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. lenmusdoc: missing mandatory element 'content'." << endl;
        parser.parse_text("<lenmusdoc vers='0.0'></lenmusdoc>");
        MyLmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pIModel->get_root() != NULL );
        CHECK( pIModel->get_root()->is_document() == true );
        CHECK( errormsg.str() == expected.str() );

        delete pIModel;
    }

    //@ lenmusdoc -----------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_parsed)
    {
        ///1.lenmusdoc: parsed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<lenmusdoc vers='0.0'><content/></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 0 );
        CHECK( pDoc->get_language() == "en" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_lenmusdoc)
    {
        ///2.lenmusdoc: id parsed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<lenmusdoc vers='0.0' id='10'><content/></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_document() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_has_content)
    {
        ///3.lenmusdoc: has content
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        string src =
            "<lenmusdoc vers='0.0'><content><para>Hello world</para></content></lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 1 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_get_content_item)
    {
        ///4.lenmusdoc: access to content
        Document doc(m_libraryScope);
        XmlParser parser;
        string src =
            "<lenmusdoc vers='0.0'><content><para>Hello world</para></content></lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_missing_vers)
    {
        ///5.lenmusdoc: no version error
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. lenmusdoc: Missing mandatory attribute 'vers'. Value '0.0' assumed." << endl;
        string src =
            "<lenmusdoc><content><para>Hello world</para></content></lenmusdoc>";
        parser.parse_text(src);
        XmlNode* tree = parser.get_tree_root();
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        CHECK( errormsg.str() == expected.str() );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( pIModel->get_root() != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_language)
    {
        ///6.lenmusdoc: has language
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<lenmusdoc vers='0.0' language='zh_CN'><content/></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        CHECK( pDoc->get_num_content_items() == 0 );
        CHECK( pDoc->get_language() == "zh_CN" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, lenmusdoc_7)
    {
        ///7.lenmusdoc: default styles created
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<lenmusdoc vers='0.0' language='zh_CN'><content/></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root() != NULL);
        CHECK( pIModel->get_root()->is_document() == true );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc != NULL );
        ImoStyle* pStyle = pDoc->find_style("Table");
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Table" );

        delete pIModel;
    }

    //@ clef ----------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_01)
    {
        //@01. clef. Created ok
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<clef><type>G</type></clef>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_clef() == true );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_02)
    {
        //@02. clef. Error in clef type. G2 assumed
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Unknown clef type 'Fa4'. Assumed 'G'." << endl;
        parser.parse_text("<clef><type>Fa4</type></clef>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_03)
    {
        //@03. clef. Location present
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<clef><type>G</type><dx>70</dx></clef>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_G2 );
        CHECK( pClef->is_visible() );
        CHECK( pClef->get_user_location_x() == 70.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_04)
    {
        //@04. clef. No visible
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<clef><type>C2</type><visible>no</visible></clef>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 0 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_05)
    {
        //@05. clef. staff 2
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<clef>"
                "<type>C2</type>"
                "<staff>2</staff>"
                "<visible>no</visible>"
            "</clef>"
        );
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_user_location_x() == 0.0f );
        CHECK( pClef->get_user_location_y() == 0.0f );
        CHECK( pClef->is_visible() == false );
        CHECK( pClef->get_staff() == 1 );
        CHECK( pClef->get_symbol_size() == k_size_default );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, clef_06)
    {
        //@06. clef. id_in_clef
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<clef id='12'>"
                "<type>C2</type>"
            "</clef>"
        );
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoClef* pClef = dynamic_cast<ImoClef*>( pIModel->get_root() );
        CHECK( pClef != NULL );
        CHECK( pClef->get_clef_type() == k_clef_C2 );
        CHECK( pClef->get_id() == 12L );

        delete pIModel;
    }


    //@ key -----------------------------------------------------------------------------

//    // instrument -----------------------------------------------------------------------
//    // time signature -------------------------------------------------------------------
//    // systemInfo -----------------------------------------------------------------------
//    // text -----------------------------------------------------------------------------
//    // metronome ------------------------------------------------------------------------
//    // opt ------------------------------------------------------------------------------
//    // nodes ----------------------------------------------------------------------------
//    // spacer ---------------------------------------------------------------------------
//    // XmlTiesBuilder ----------------------------------------------------------------------
//    // beam -----------------------------------------------------------------------------
//    // XmlAutoBeamer -----------------------------------------------------------------------
//    // tuplet new syntax ----------------------------------------------------------------
//    // voice (element) ------------------------------------------------------------------
//    // staffNum (element) ---------------------------------------------------------------
//    // rest (full) ----------------------------------------------------------------------

    //@ color ---------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, ImoColorDto)
    {
        ImoColorDto color;
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
        color.set_from_rgb_string("#ff3217");
        CHECK( color.red() == 255 );
        CHECK( color.green() == 50 );
        CHECK( color.blue() == 23 );
        CHECK( color.alpha() == 255 );
        color.set_from_rgba_string("#fffe4580");
        CHECK( color.red() == 255 );
        CHECK( color.green() == 254 );
        CHECK( color.blue() == 69 );
        CHECK( color.alpha() == 128 );
        CHECK( color.is_color_dto() == true );
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, ImoColor_Constructor)
    {
        ImoColorDto color(12,32,255,180);
        CHECK( color.red() == 12 );
        CHECK( color.green() == 32 );
        CHECK( color.blue() == 255 );
        CHECK( color.alpha() == 180 );
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, ImoColor_Error)
    {
        ImoColorDto color;
        color.set_from_string("#ff3g17");
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
        color.set_from_string("#fff");
        CHECK( color.red() == 0 );
        CHECK( color.green() == 0 );
        CHECK( color.blue() == 0 );
        CHECK( color.alpha() == 255 );
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Analyser_Color_ErrorInvalidData)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Missing or invalid color value. Must be #rrggbbaa. Color ignored." << endl;
        parser.parse_text("<color>321700</color>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root() == NULL );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Analyser_Color_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<color>#f0457f</color>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoColorDto* pColor = dynamic_cast<ImoColorDto*>( pIModel->get_root() );
        CHECK( pColor != NULL );
        CHECK( pColor->red() == 240 );
        CHECK( pColor->green() == 69 );
        CHECK( pColor->blue() == 127 );
        CHECK( pColor->alpha() == 255 );
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        delete pIModel;
    }

//    // group ----------------------------------------------------------------------------
//    // chord ----------------------------------------------------------------------------
//    // pageLayout -----------------------------------------------------------------------

    //@ defineStyle ---------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, DefineStyle_FontProperties)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<defineStyle>"
                "<name>Composer</name>"
                "<font-name>Arial</font-name>"
                "<font-size>14pt</font-size>"
                "<font-style>italic</font-style>"
                "<font-weight>bold</font-weight>"
                "<font-file>FreeSans.ttf</font-file>"
            "</defineStyle>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyle* pStyle = dynamic_cast<ImoStyle*>( pIModel->get_root() );
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Composer" );
        CHECK( pStyle->font_name() == "Arial" );
        CHECK( pStyle->font_file() == "FreeSans.ttf" );
        CHECK( pStyle->font_size() == 14 );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );

        delete pIModel;
    }

//    // title ----------------------------------------------------------------------------
//    // line -----------------------------------------------------------------------------
//    // textBox --------------------------------------------------------------------------
//    // cursor ---------------------------------------------------------------------------
//    // figuredBass ----------------------------------------------------------------------
//    // staff ----------------------------------------------------------------------------

    //@ textItem ------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, TextItem)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<txt>This is a text</txt>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "This is a text" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_text_item)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<txt id='10'>This is a text</txt>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, TextItem_MissingText)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. txt: missing mandatory value 'string'. Element <txt> ignored." << endl;
        parser.parse_text("<txt></txt>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText == NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, TextItem_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Title</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><para><txt style='Title'>This is a text</txt></para></content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == "This is a text" );
        ImoStyle* pStyle = pText->get_style();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Title" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, TextItem_whitespace_collapsed)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<txt> This is a         text        </txt>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == " This is a text " );
        //cout << "result: \"" << pText->get_text() << "\"";

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, TextItem_whitespace_normal)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<txt> This is a\n         text\nwith newlines\n        </txt>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_text_item() == true );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pIModel->get_root() );
        CHECK( pText != NULL );
        CHECK( pText->get_text() == " This is a text with newlines " );
        //cout << "result: \"" << pText->get_text() << "\"" << endl;

        delete pIModel;
    }

    //@ para ----------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, paragraph_creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src = "<para>This is a paragraph</para>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_paragraph() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_paragraph)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<para id='10'>This is a paragraph</para>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_paragraph() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_TextItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<para>This is a paragraph</para>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "This is a paragraph" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_anonymous_text_whitespace)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<para>\n      This is a\n      paragraph\n       </para>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == " This is a paragraph " );
        //cout << "7102-result: \"" << pItem->get_text() << "\"" << endl;

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_LinkItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<para><link url='This is the url'>This is the link</link></para>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 1 );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "This is the url" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "This is the link" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_link)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<link id='10' url='This is the url'>This is the link</link>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_link() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_ManyItems)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src = "<para>This is a paragraph <txt style='bold'>with two items.</txt></para>";
        parser.parse_text(src);
        //parser.parse_text("<para>(txt \"This is a paragraph\")"
        //    "(txt \" with two items.\") )");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pIModel->get_root() );
        CHECK( pPara->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pPara->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == "This is a paragraph " );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == "with two items." );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<para>Hello world!</para></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Credits</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><para style='Credits'>Hello world!</para></content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Credits" );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_DefaultStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<para>Hello world!</para></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style();
        CHECK( pStyle->get_name() == "Paragraph" );
        CHECK( pPara->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pPara->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Paragraph_NoStyle)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<para>Hello world!</para></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        ImoStyle* pStyle = pPara->get_style(false);
        CHECK( pStyle == NULL );

        delete pIModel;
    }

    //@ section -------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<section level='1'>This is a header</section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_heading() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_section)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<section id='10' level='1'>This is a header</section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_heading() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_TextItemAdded)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<section level='3'>This is a header</section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 1 );
        CHECK( pH->get_level() == 3 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "This is a header" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_no_level)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. section: missing 'level' attribute. Level 1 assumed." << endl;
        parser.parse_text("<section>This is a header</section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 1 );
        CHECK( pH->get_level() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "This is a header" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_invalid_level)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Invalid integer number 'apple'. Replaced by '1'." << endl;
        parser.parse_text("<section level='apple'>This is a header</section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 1 );
        CHECK( pH->get_level() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "This is a header" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_ManyItems)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<section level='1'>This is a header"
            "<txt> with two items.</txt></section>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pIModel->get_root() );
        CHECK( pH->get_num_items() == 2 );
        TreeNode<ImoObj>::children_iterator it = pH->begin();
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == "This is a header" );
        ++it;
        pItem = dynamic_cast<ImoTextItem*>( *it );
        CHECK( pItem->get_text() == " with two items." );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_RecognizedAsContent)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<section level='1'>Hello world!</section></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != NULL );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Section_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Title</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><section level='1' style='Heading'>Hello world!</section></content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoHeading* pH = dynamic_cast<ImoHeading*>( pDoc->get_content_item(0) );
        CHECK( pH != NULL );
        ImoStyle* pStyle = pH->get_style();
        CHECK( pStyle != NULL );
        CHECK( pH->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pH->get_first_item() );
        CHECK( pItem->get_text() == "Hello world!" );

        delete pIModel;
    }

    //@ styles --------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, Styles)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
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
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
        CHECK( pStyles != NULL );

        ImoStyle* pStyle = pStyles->find_style("Header1");
        CHECK( pStyle->get_name() == "Header1" );
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Styles_all_simple_properties)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<styles>"
                "<defineStyle>"
                    "<name>Header1</name>"
                        // color and background
                    "<color>#00fe0f7f</color>"
                    "<background-color>#00fc0c7c</background-color>"
                        // font
                    "<font-name>Times New Roman</font-name>"
                    "<font-size>14pt</font-size>"
                    "<font-style>italic</font-style>"
                    "<font-weight>bold</font-weight>"
                        // border width
                    "<border-width-top>20</border-width-top>"
                    "<border-width-right>21</border-width-right>"
                    "<border-width-bottom>22</border-width-bottom>"
                    "<border-width-left>23</border-width-left>"
                        // margin
                    "<margin-top>24</margin-top>"
                    "<margin-right>25</margin-right>"
                    "<margin-bottom>26</margin-bottom>"
                    "<margin-left>27</margin-left>"
                        // padding
                    "<padding-top>28</padding-top>"
                    "<padding-right>29</padding-right>"
                    "<padding-bottom>30</padding-bottom>"
                    "<padding-left>31</padding-left>"
                        //text
                    "<text-decoration>overline</text-decoration>"
                    "<vertical-align>text-top</vertical-align>"
                    "<text-align>center</text-align>"
                    "<line-height>77</line-height>"
                        //size
                    "<min-height>32</min-height>"
                    "<min-width>33</min-width>"
                    "<max-height>34</max-height>"
                    "<max-width>35</max-width>"
                    "<height>36</height>"
                    "<width>37</width>"
                        //table
                    "<table-col-width>38</table-col-width>"
                "</defineStyle>"
            "</styles>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
        CHECK( pStyles != NULL );

        ImoStyle* pStyle = pStyles->find_style("Header1");
        CHECK( pStyle->get_name() == "Header1" );
            // color and background
        CHECK( is_equal(pStyle->color(), Color(0, 254,15, 127)) );
        CHECK( is_equal(pStyle->background_color(), Color(0, 252 ,12, 124)) );
            // font
        CHECK( pStyle->font_name() == "Times New Roman" );
        CHECK( pStyle->font_style() == ImoStyle::k_font_style_italic );
        CHECK( pStyle->font_weight() == ImoStyle::k_font_weight_bold );
        CHECK( pStyle->font_size() == 14 );
            // border width
        CHECK( pStyle->border_width_top() == 20.0f );
        CHECK( pStyle->border_width_right() == 21.0f );
        CHECK( pStyle->border_width_bottom() == 22.0f );
        CHECK( pStyle->border_width_left() == 23.0f );
            // margin
        CHECK( pStyle->margin_top() == 24.0f );
        CHECK( pStyle->margin_right() == 25.0f );
        CHECK( pStyle->margin_bottom() == 26.0f );
        CHECK( pStyle->margin_left() == 27.0f );
            // padding
        CHECK( pStyle->padding_top() == 28.0f );
        CHECK( pStyle->padding_right() == 29.0f );
        CHECK( pStyle->padding_bottom() == 30.0f );
        CHECK( pStyle->padding_left() == 31.0f );
            //text
        CHECK( pStyle->text_decoration() == ImoStyle::k_decoration_overline );
        CHECK( pStyle->vertical_align() == ImoStyle::k_valign_text_top );
        CHECK( pStyle->text_align() == ImoStyle::k_align_center );
        CHECK( pStyle->line_height() == 77.0f );
            //size
        CHECK( pStyle->min_height() == 32.0f );
        CHECK( pStyle->min_width() == 33.0f );
        CHECK( pStyle->max_height() == 34.0f );
        CHECK( pStyle->max_width() == 35.0f );
        CHECK( pStyle->height() == 36.0f );
        CHECK( pStyle->width() == 37.0f );
            //table
        CHECK( pStyle->table_col_width() == 38.0f );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Styles_all_compound_properties)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<styles>"
                "<defineStyle>"
                    "<name>Header1</name>"
                    "<border-width>22.33</border-width>"
                    "<margin>15.5</margin>"
                    "<padding>12</padding>"
                "</defineStyle>"
            "</styles>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoStyles* pStyles = dynamic_cast<ImoStyles*>( pIModel->get_root() );
        CHECK( pStyles != NULL );

        ImoStyle* pStyle = pStyles->find_style("Header1");
        CHECK( pStyle->get_name() == "Header1" );
            // border width
        CHECK( pStyle->border_width_top() == 22.33f );
        CHECK( pStyle->border_width_right() == 22.33f );
        CHECK( pStyle->border_width_bottom() == 22.33f );
        CHECK( pStyle->border_width_left() == 22.33f );
            // margin
        CHECK( pStyle->margin_top() == 15.5f );
        CHECK( pStyle->margin_right() == 15.5f );
        CHECK( pStyle->margin_bottom() == 15.5f );
        CHECK( pStyle->margin_left() == 15.5f );
            // padding
        CHECK( pStyle->padding_top() == 12.0f );
        CHECK( pStyle->padding_right() == 12.0f );
        CHECK( pStyle->padding_bottom() == 12.0f );
        CHECK( pStyle->padding_left() == 12.0f );

        delete pIModel;
    }

    //@ param ---------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, ParamInfo_Ok)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<param name='green'>this is green</param>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        CHECK( pIModel->get_root()->is_param_info() == true );
        ImoParamInfo* pParam = dynamic_cast<ImoParamInfo*>( pIModel->get_root() );
        CHECK( pParam != NULL );
        CHECK( pParam->get_name() == "green" );
        CHECK( pParam->get_value() == "this is green" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, ParamInfo_MissingName)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. Missing name for element 'param'. Element ignored." << endl;
        parser.parse_text("<param>this is green</param>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root() == NULL );

        delete pIModel;
    }


    //@ dynamic -------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, Dynamic_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<dynamic classid='test' />");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_dynamic() == true );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
        CHECK( pDyn != NULL );
        CHECK( pDyn->get_classid() == "test" );
        CHECK( pDyn->is_visible() );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Dynamic_AddedToContent)
    {
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<dynamic classid='test' /></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pDoc->get_content_item(0) );
        CHECK( pDyn != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_dynamic)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<dynamic id='10' classid='test'>"
                "<param name='play'>all notes</param>"
            "</dynamic>"
        );
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->is_dynamic() == true );
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Dynamic_GeneratesRequest)
    {
        LomseDoorway* pDoorway = m_libraryScope.platform_interface();
        pDoorway->set_request_callback(this, wrapper_lomse_request);

        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<lenmusdoc vers='0.0'><content>"
            "<dynamic classid='test' /></content></lenmusdoc>");
        LmdAnalyser a(cout, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );

        CHECK( m_fRequestReceived == true );
        CHECK( m_requestType == k_dynamic_content_request );
        CHECK( m_pDoc == pDoc );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Dynamic_WithParams)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<dynamic classid='test'>"
                "<param name='play'>all notes</param>"
            "</dynamic>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDynamic* pDyn = dynamic_cast<ImoDynamic*>( pIModel->get_root() );
        CHECK( pDyn->get_classid() == "test" );
        std::list<ImoParamInfo*>& params = pDyn->get_params();
        CHECK( params.size() == 1 );
        ImoParamInfo* pParm = params.front();
        CHECK( pParm->get_name() == "play" );
        CHECK( pParm->get_value() == "all notes" );

        delete pIModel;
    }

    //@ link ----------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, Link_Ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<link url='#TheoryHarmony_ch3.lms'>Harmony exercise</link>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "#TheoryHarmony_ch3.lms" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "Harmony exercise" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Link_MissingUrl)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. link: Missing mandatory attribute 'url'." << endl;
        parser.parse_text(
            "<link>Harmony exercise</link>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_link() == true );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pIModel->get_root() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "" );
        CHECK( pLink->get_num_items() == 1 );
        ImoTextItem* pItem = dynamic_cast<ImoTextItem*>( pLink->get_first_item() );
        CHECK( pItem->get_text() == "Harmony exercise" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, Link_Style)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>Button</name><color>#00fe0f7f</color></defineStyle>"
                "</styles>"
                "<content><para>"
                    "<link style='Button' url='#TheoryHarmony_ch3.lms'>Harmony exercise</link>"
                "</para></content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        CHECK( pDoc->get_num_content_items() == 1 );
        ImoParagraph* pPara = dynamic_cast<ImoParagraph*>( pDoc->get_content_item(0) );
        CHECK( pPara != NULL );
        CHECK( pPara->get_num_items() == 1 );
        ImoLink* pLink = dynamic_cast<ImoLink*>( pPara->get_first_item() );
        CHECK( pLink != NULL );
        CHECK( pLink->get_url() == "#TheoryHarmony_ch3.lms" );
        ImoStyle* pStyle = pLink->get_style();
        CHECK( pStyle != NULL );
        CHECK( pStyle->get_name() == "Button" );

        delete pIModel;
    }

////    // image ----------------------------------------------------------------------------

    //@ list ----------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, Listitem_created)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "<listitem>This is the first item</listitem>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_listitem() == true );
        ImoListItem* pLI = dynamic_cast<ImoListItem*>( pIModel->get_root() );
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is the first item" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_listitem)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<listitem id='10'>This is the first item</listitem>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_listitem() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, List_created)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "<itemizedlist>"
                "<listitem>This is the first item</listitem>"
                "<listitem>This is the second item</listitem>"
            "</itemizedlist>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_list() == true );
        ImoList* pList = dynamic_cast<ImoList*>( pIModel->get_root() );
        CHECK( pList != NULL );
        CHECK( pList->get_list_type() == ImoList::k_itemized );
        CHECK( pList->get_num_content_items() == 2 );
        ImoListItem* pLI = pList->get_list_item(0);
        CHECK( pLI->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pLI->get_content_item(0) );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is the first item" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_list)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<itemizedlist id='10'>"
                "<listitem>This is the first item</listitem>"
                "<listitem>This is the second item</listitem>"
            "</itemizedlist>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_list() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

//    // graphic line  --------------------------------------------------------------------

    //@ scorePlayer ---------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, scorePlayer_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text("<lenmusdoc vers='0.0'> <content>"
            "<scorePlayer/></content></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 60 );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_score_player)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text("<lenmusdoc vers='0.0'> <content>"
            "<scorePlayer id='10'/></content></lenmusdoc>");
        //parser.parse_text("<scorePlayer id='10' />");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, scorePlayer_metronome)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<lenmusdoc vers='0.0'> <content>"
                "<scorePlayer><mm>65</mm></scorePlayer>"
            "</content></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 65 );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, scorePlayer_label_play)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<lenmusdoc vers='0.0'> <content>"
                "<scorePlayer>"
                    "<playLabel>Tocar</playLabel>"
                    "<stopLabel>Parar</stopLabel>"
                    "<mm>65</mm>"
                "</scorePlayer>"
            "</content></lenmusdoc>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pDoc->get_content_item(0) );
        ImoScorePlayer* pSP = dynamic_cast<ImoScorePlayer*>( pAB->get_first_item() );
        CHECK( pSP->is_score_player() == true );
        CHECK( pSP->get_metronome_mm() == 65 );
        CHECK( pSP->get_play_label() == "Tocar" );
        CHECK( pSP->get_stop_label() == "Parar" );
        //cout << "metronome mm = " << pSP->get_metronome_mm() << endl;

        delete pIModel;
    }

    //@ tableCell -----------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, tableCell_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<tableCell>This is a cell</tableCell>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_table_cell)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<tableCell id='10'>This is a cell</tableCell>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_cell() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, tableCell_rowspan)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<tableCell><rowspan>2</rowspan>This is a cell</tableCell>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 2 );
        CHECK( pCell->get_colspan() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, tableCell_colspan)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<tableCell><colspan>2</colspan>This is a cell</tableCell>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        CHECK( pCell->get_rowspan() == 1 );
        CHECK( pCell->get_colspan() == 2 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == "This is a cell" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, tableCell_whitespace)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<tableCell>\n    This is\n   a cell\n    </tableCell>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableCell* pCell = dynamic_cast<ImoTableCell*>( pIModel->get_root() );
        CHECK( pCell->is_table_cell() == true );
        CHECK( pCell->get_num_content_items() == 1 );
        ImoAnonymousBlock* pAB = dynamic_cast<ImoAnonymousBlock*>( pCell->get_content_item(0) );
        CHECK( pAB->get_num_items() == 1 );
        ImoTextItem* pText = dynamic_cast<ImoTextItem*>( pAB->get_first_item() );
        CHECK( pText->get_text() == " This is a cell " );
        //cout << "8462-result: \"" << pText->get_text() << "\"" << endl;

        delete pIModel;
    }

    //@ tableRow ------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, tableRow_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<tableRow>"
                "<tableCell>This is cell 1</tableCell>"
                "<tableCell>This is cell 2</tableCell>"
            "</tableRow>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pIModel->get_root() );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 2 );
        ImoTableCell* pImo = dynamic_cast<ImoTableCell*>( pRow->get_cell(0) );
        CHECK( pImo->is_table_cell() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_table_row)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<tableRow id='10'>"
                "<tableCell>This is cell 1</tableCell>"
                "<tableCell>This is cell 2</tableCell>"
            "</tableRow>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_row() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    //@ tableHead -----------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, tableHead_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<tableHead>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableHead>";;
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableHead* pHead = dynamic_cast<ImoTableHead*>( pIModel->get_root() );
        CHECK( pHead->is_table_head() == true );
        CHECK( pHead->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );
        CHECK( pRow->get_num_cells() == 1 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_table_head)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<tableHead id='10'>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableHead>" );
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_head() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    //@ tableBody -----------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, tableBody_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<tableBody>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableBody>";;
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTableBody* pBody = dynamic_cast<ImoTableBody*>( pIModel->get_root() );
        CHECK( pBody->is_table_body() == true );
        CHECK( pBody->get_num_items() == 2 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_table_body)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<tableBody id='10'>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableBody>" );
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table_body() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    //@ table ---------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, table_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<table><tableBody>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableBody></table>";;
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pIModel->get_root() );
        CHECK( pTable != NULL );

        CHECK( pTable->get_head() == NULL );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != NULL );
        CHECK( pBody->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, id_in_table)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "" << endl;
        parser.parse_text(
            "<table id='10'><tableBody>"
                "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
            "</tableBody></table>" );
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");
//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        CHECK( pIModel->get_root()->is_table() == true );
        ImoObj* pImo = pIModel->get_root();
        CHECK( pImo->get_id() == 10L );

        delete pIModel;
    }

    //@ tableColumn ---------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, tableColumn_Creation)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
                "</styles>"
                "<content>"
                    "<table>"
                        "<tableColumn style='table1-col1' />"
                        "<tableColumn style='table1-col2' />"
                        "<tableBody>"
                            "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                        "</tableBody>"
                    "</table>"
                "</content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != NULL );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );
        std::list<ImoStyle*>::iterator it = cols.begin();
        CHECK( (*it)->get_name() == "table1-col1" );
        ++it;
        CHECK( (*it)->get_name() == "table1-col2" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, table_full_table)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
                "</styles>"
                "<content>"
                    "<table>"
                        "<tableColumn style='table1-col1' />"
                        "<tableColumn style='table1-col2' />"
                        "<tableHead>"
                            "<tableRow>"
                                "<tableCell>This is head cell 1</tableCell>"
                                "<tableCell>This is head cell 2</tableCell>"
                            "</tableRow>"
                        "</tableHead>"
                        "<tableBody>"
                            "<tableRow>"
                                "<tableCell>This is body cell 1</tableCell>"
                                "<tableCell>This is body cell 2</tableCell>"
                            "</tableRow>"
                        "</tableBody>"
                    "</table>"
                "</content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != NULL );

        std::list<ImoStyle*>& cols = pTable->get_column_styles();
        CHECK( cols.size() == 2 );

        ImoTableHead* pHead = pTable->get_head();
        CHECK( pHead != NULL );
        CHECK( pHead->get_num_items() == 1 );
        ImoTableRow* pRow = dynamic_cast<ImoTableRow*>( pHead->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        ImoTableBody* pBody = pTable->get_body();
        CHECK( pBody != NULL );
        CHECK( pBody->get_num_items() == 1 );
        pRow = dynamic_cast<ImoTableRow*>( pBody->get_item(0) );
        CHECK( pRow->is_table_row() == true );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, table_added_to_content)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
                "</styles>"
                "<content>"
                    "<table>"
                        "<tableColumn style='table1-col1' />"
                        "<tableColumn style='table1-col2' />"
                        "<tableBody>"
                            "<tableRow><tableCell>This is a cell</tableCell></tableRow>"
                        "</tableBody>"
                    "</table>"
                "</content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoTable* pTable = dynamic_cast<ImoTable*>( pDoc->get_content_item(0) );
        CHECK( pTable != NULL );

        delete pIModel;
    }

    //@ ldpmusic ------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, ldpmusic_ok)
    {
        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        string src =
            "<lenmusdoc vers='0.0'>"
                "<styles>"
                    "<defineStyle><name>table1-col1</name><width>70</width></defineStyle>"
                    "<defineStyle><name>table1-col2</name><width>80</width></defineStyle>"
                "</styles>"
                "<content>"
                    "<ldpmusic>"
                        "(score (vers 1.6)(instrument (musicData (clef G)(n c4 q.))))"
                    "</ldpmusic>"
                "</content>"
            "</lenmusdoc>";
        parser.parse_text(src);
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoDocument* pDoc = dynamic_cast<ImoDocument*>( pIModel->get_root() );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        CHECK( pScore != NULL );

        delete pIModel;
    }

    //@ score ---------------------------------------------------------------------------

    TEST_FIXTURE(LmdAnalyserTestFixture, score_00)
    {
        //00. minimun score ok: has version and one instrument.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. link: missing mandatory element 'url'." << endl;
        parser.parse_text(
            "<score><instrument><musicData/></instrument></score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore != NULL );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_version_string() == "2.0" );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_20)
    {
        //20. parts defined. error: instrument requires <instrId>

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. instrument: missing 'instrId' element. Instrument ignored."
                 << endl;
        parser.parse_text(
            "<score>"
            "<parts><instrId>P1</instrId></parts>"
            "<instrument>"
                "<musicData/>"
            "</instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_21)
    {
        //21. parts, minimum content: one instr id.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<score>"
            "<parts><instrId>P1</instrId></parts>"
            "<instrument>"
                "<instrId>P1</instrId>"
                "<musicData/>"
            "</instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_22)
    {
        //22. error: <instrId> not defined in <parts>

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. 'instrId' is not defined in <parts> element. Instrument ignored."
                 << endl;
        parser.parse_text(
            "<score>"
            "<parts><instrId>P1</instrId></parts>"
            "<instrument>"
                "<instrId>P2</instrId>"
                "<musicData/>"
            "</instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_23)
    {
        //23. parts. error: duplicated <instrId>

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. parts: duplicated <instrId> will be ignored."
                 << endl;
        parser.parse_text(
            "<score>"
            "<parts>"
                "<instrId>P1</instrId>"
                "<instrId>P1</instrId>"
            "</parts>"
            "<instrument>"
                "<instrId>P1</instrId>"
                "<musicData/>"
            "</instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );
        CHECK( pScore->get_instrument("P1") != NULL );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_24)
    {
        //24. parts, error: at least on <instrId> is required

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        expected << "Line 0. parts: at least one <instrId> is required." << endl;
        parser.parse_text(
            "<score>"
            "<parts></parts>"
            "<instrument><musicData/></instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 1 );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_25)
    {
        //25. parts, with one group. ok.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<score>"
            "<parts>"
                "<instrId>P1</instrId>"
                "<instrId>P2</instrId>"
                "<group><firstInstr>P1</firstInstr><lastInstr>P2</lastInstr></group>"
            "</parts>"
            "<instrument><instrId>P1</instrId><musicData/></instrument>"
            "<instrument><instrId>P2</instrId><musicData/></instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_none );

        delete pIModel;
    }

    TEST_FIXTURE(LmdAnalyserTestFixture, score_26)
    {
        //26. parts, one group with grpSymbol.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        stringstream expected;
        //expected << "Line 0. " << endl;
        parser.parse_text(
            "<score>"
            "<parts>"
                "<instrId>P1</instrId>"
                "<instrId>P2</instrId>"
                "<group>"
                    "<firstInstr>P1</firstInstr><lastInstr>P2</lastInstr>"
                    "<grpSymbol>bracket</grpSymbol>"
                "</group>"
            "</parts>"
            "<instrument><instrId>P1</instrId><musicData/></instrument>"
            "<instrument><instrId>P2</instrId><musicData/></instrument>"
            "</score>");
        LmdAnalyser a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        InternalModel* pIModel = a.analyse_tree(tree, "string:");

//        cout << "[" << errormsg.str() << "]" << endl;
//        cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );

        CHECK( pIModel->get_root()->is_score() == true );
        ImoScore* pScore = dynamic_cast<ImoScore*>( pIModel->get_root() );
        CHECK( pScore->get_num_instruments() == 2 );
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        CHECK( pGroups != NULL );
        ImoInstrGroup* pGroup = dynamic_cast<ImoInstrGroup*>( pGroups->get_first_child() );
        CHECK( pGroup != NULL );
        CHECK( pGroup->get_instrument(0) != NULL );
        CHECK( pGroup->get_instrument(1) != NULL );
        CHECK( pGroup->get_abbrev_string() == "" );
        CHECK( pGroup->get_name_string() == "" );
        CHECK( pGroup->get_symbol() == ImoInstrGroup::k_bracket );

        delete pIModel;
    }

}

