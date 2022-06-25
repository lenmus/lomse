//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_mxl_exporter.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "private/lomse_document_p.h"
#include "lomse_im_factory.h"
#include "lomse_staffobjs_table.h"
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;



//=======================================================================================
// test for MxlExporter
//=======================================================================================

//---------------------------------------------------------------------------------------
// access to protected members
class MyMxlAnalyser2 : public MxlAnalyser
{
public:
    MyMxlAnalyser2(ostream& reporter, LibraryScope& libScope, Document* pDoc,
                   XmlParser* parser)
        : MxlAnalyser(reporter, libScope, pDoc, parser)
    {
    }

    void do_not_delete_instruments_in_destructor()
    {
        m_partList.do_not_delete_instruments_in_destructor();
    }
};

class MxlExporterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MxlExporterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~MxlExporterTestFixture()    //TearDown fixture
    {
    }

    void dump_colection(ImoScore* pScore)
    {
        ColStaffObjs* pCol = pScore->get_staffobjs_table();
        cout << pCol->dump();
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    bool check_result(const string& ss, const string& expected)
    {
        if (ss != expected)
        {
            cout << "  result=[" << ss << "]" << endl;
            cout << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_result_contains(const string& ss, const string& expected, const string& tag)
    {
        if (ss.find(expected) == std::string::npos)
        {
            size_t iStart = ss.find("<"+tag);
            if(iStart == std::string::npos)
                cout << "  result=[" << ss << "]" << endl;
            else
            {
                size_t iEnd = ss.rfind("</"+tag);
                if(iEnd == std::string::npos)
                    cout << "  result=[" << ss.substr(iStart) << "]" << endl;
                else
                {
                    iEnd += string("</"+tag+">").length();
                    cout << "  result=[" << ss.substr(iStart, iEnd-iStart) << "]" << endl;
                }
            }
            cout << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

};

//---------------------------------------------------------------------------------------
SUITE(MxlExporterTest)
{
    //@ develop -------------------------------------------------------------------------

//    TEST_FIXTURE(MxlExporterTestFixture, develop_01)
//    {
//        //@01. export generated score, to test round import
//
//        Document doc(m_libraryScope);
////        doc.from_string("(score (vers 2.0) (instrument (musicData "
////            "(r q)(barline startRepetition)(r q)(barline endRepetition))))");
//        //doc.from_file(m_scores_path + "01020-beams.lms" );
//        //doc.from_file(m_scores_path + "01026-beamed-chords.lms" );
//        //doc.from_file(m_scores_path + "01024-rests-in-beam.lms" );
//        //doc.from_file(m_scores_path + "01030-ties.lms" );
//        //doc.from_file(m_scores_path + "01010-tuplet-triplets.lms" );
//        //doc.from_file(m_scores_path + "01014-nested-tuplets.lms" );
//        //doc.from_file(m_scores_path + "00086-chord-notes-ordering.lms" );
//        //doc.from_file(m_scores_path + "unit-tests/xml-export/101-forward.xml", Document::k_format_mxl);
//
//        //doc.from_file(m_scores_path + "01040-slur.lms" );
//        //doc.from_file(m_scores_path + "01031-tie-bezier.lms" );
//        //02090-lyrics-two-lines-only-text
//        //02091-lyrics-melisma-hyphenation
//        //02093-lyrics-above-below
//        //02021-all-fermatas
//        //02030-metronome
//        //02070-dynamics-marks
//        //02080-all-accents
//        //02081-all-caesura-and-breath-marks
//        //Lilypond tests
//        //02b-Rests-PitchedRests.xml", Document::k_format_mxl);
//        //12a-Clefs.xml", Document::k_format_mxl);
//        //doc.from_file(m_scores_path + "unit-tests/xml-export/45e-Repeats-Nested-Alternatives.xml", Document::k_format_mxl);
//        //doc.from_file(m_scores_path + "unit-tests/xml-export/MozartTrio.xml", Document::k_format_mxl);
//        doc.from_file(m_scores_path + "unit-tests/xml-export/BeetAnGeSample.xml", Document::k_format_mxl);
//        //doc.from_file(m_scores_path + "unit-tests/xml-export/003-slur.xml", Document::k_format_mxl);
//        //doc.from_file(m_scores_path + "00205-multimetric.lmd", Document::k_format_lmd );
//        //doc.from_file(m_scores_path + "00023-spacing-in-prolog-two-instr.lms" );
////        doc.from_file(m_scores_path + "50120-fermatas.lms" );
////        doc.from_file(m_scores_path + "50051-tie-bezier.lms" );
////        doc.from_file(m_scores_path + "00110-triplet-against-5-tuplet-4.14.lms" );
////        doc.from_file(m_scores_path + "50130-metronome.lms" );
////        doc.from_file(m_scores_path + "50180-new-system-tag.lms" );
////        doc.from_file(m_scores_path + "50110-graphic-line-text.lms" );
////        doc.from_file("/datos/cecilio/Desarrollo_wx/lomse/samples/chopin_prelude20_v16.lms" );
//        //doc.from_file(m_scores_path + "MahlFaGe4Sample.mxl,
//        ImoDocument* pRoot = doc.get_im_root();
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_add_id(true);
//
//        ofstream file1(m_scores_path + "../z_test_musicxml_export.xml", ios::out);
//        if (file1.good())
//        {
//            string source = exporter.get_source(pRoot);
////            cout << "----------------------------------------------------" << endl;
////            cout << source << endl;
////            cout << "----------------------------------------------------" << endl;
//            file1.write(source.c_str(), source.size());
//            file1.close();
//        }
//        else
//        {
//            std::cout << "file error write" << endl;
//            CHECK( false );
//        }
//    }

    //@ articulations -------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, articulations_01)
    {
        //@01. articulation symbol. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 q (staccato))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //first note

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><articulations><staccato/></articulations>"
            "</notations></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, articulations_02)
    {
        //@02. articulations. placement

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 q (tenuto above))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //first note

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><articulations><tenuto placement=\"above\"/></articulations>"
            "</notations></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, articulations_03)
    {
        //@03. articulation line

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<articulations><plop placement=\"below\"/></articulations>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<notations><articulations><plop placement=\"below\"/></articulations>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    //@ attributes ----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, attributes_01)
    {
        //@01. attributes added to first measure in right order

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)"
            "(key A)(time 3 4)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\"><attributes><divisions>480</divisions>"
            "<key><fifths>3</fifths><mode>major</mode></key>"
            "<time><beats>3</beats><beat-type>4</beat-type></time>"
            "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes></measure>";
        CHECK( check_result(source, expected) );
    }

    //@ backup --------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, backup_01)
    {
        //@01. two voices.

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 q v1)(n e4 q v1)(n a4 q v2)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
                "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>480</duration>"
                "<voice>1</voice><type>quarter</type></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch><duration>480</duration>"
                "<voice>1</voice><type>quarter</type></note>"
            "<backup><duration>960</duration></backup>"
            "<note><pitch><step>A</step><octave>4</octave></pitch><duration>480</duration>"
                "<voice>2</voice><type>quarter</type></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    //@ barline -------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, barline_00)
    {
        //@00. barline. end barline

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData (r q)(barline end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions></attributes>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_01)
    {
        //@01. barline. regular barline

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData (r q)(barline))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions></attributes>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_02)
    {
        //@02. barline. start repetition

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(r q)(barline startRepetition)(r q)(barline endRepetition))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions></attributes>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>"
            "<measure number=\"2\">"
            "<barline location=\"left\"><bar-style>heavy-light</bar-style>"
            "<repeat direction=\"forward\"/></barline>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style>"
            "<repeat direction=\"backward\"/></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_03)
    {
        //@03. barline. double repetition

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(r q)(barline startRepetition)(r q)(barline doubleRepetition)"
            "(r q)(barline endRepetition))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions></attributes>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>"
            "<measure number=\"2\">"
            "<barline location=\"left\"><bar-style>heavy-light</bar-style>"
            "<repeat direction=\"forward\"/></barline>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style>"
            "<repeat direction=\"backward\"/></barline>"
            "</measure>"
            "<measure number=\"3\">"
            "<barline location=\"left\"><bar-style>heavy-light</bar-style>"
            "<repeat direction=\"forward\"/></barline>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style>"
            "<repeat direction=\"backward\"/></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_04)
    {
        //@04. barline. no visible

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData "
            "(r q)(barline simple noVisible)(r q)(barline end))))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions></attributes>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>none</bar-style></barline>"
            "</measure>"
            "<measure number=\"2\">"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    // @ beam ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, beam_00)
    {
        //@00. beam. begin and end

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(n c4 e g+)(n e4 e g-) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //note c4

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>240</duration><voice>1</voice><type>eighth</type>"
            "<beam number=\"1\">begin</beam></note>";
        CHECK( check_result(source, expected) );

        ++it;   //note E4
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>E</step><octave>4</octave></pitch>"
            "<duration>240</duration><voice>1</voice><type>eighth</type>"
            "<beam number=\"1\">end</beam></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, beam_01)
    {
        //@01. chord: beam only in base note. backward hook

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)"
            "(chord (n e4 e. g+ (stem up))(n g4 e.))"
            "(chord (n d4 s g- (stem up))(n f4 s))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //note e4

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>E</step><octave>4</octave></pitch>"
            "<duration>360</duration><voice>1</voice><type>eighth</type>"
            "<dot/><stem>up</stem><beam number=\"1\">begin</beam></note>";
        CHECK( check_result(source, expected) );

        ++it;   //note g4
        source = exporter.get_source(*it);
        expected = "<note><chord/><pitch><step>G</step><octave>4</octave></pitch>"
            "<duration>360</duration><voice>1</voice><type>eighth</type><dot/>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //note d4
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>D</step><octave>4</octave></pitch>"
            "<duration>120</duration><voice>1</voice><type>16th</type>"
            "<stem>up</stem><beam number=\"1\">end</beam>"
            "<beam number=\"2\">backward hook</beam></note>";
        CHECK( check_result(source, expected) );

        ++it;   //note f4
        source = exporter.get_source(*it);
        expected = "<note><chord/><pitch><step>F</step><octave>4</octave></pitch>"
            "<duration>120</duration><voice>1</voice><type>16th</type>"
            "</note>";
        CHECK( check_result(source, expected) );
    }

    //@ clef ----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, clef_00)
    {
        //@00 clef, minimal test. sign and line

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pClef);
        string expected = "<clef><sign>F</sign><line>4</line></clef>";
        CHECK( check_result(source, expected) );
        delete pClef;
    }

    TEST_FIXTURE(MxlExporterTestFixture, clef_01)
    {
        //@01. staff added when instrument has more than one

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (staves 2)"
                "(musicData (clef G p1)(clef F4 p2)(n d4 q p2) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<clef number=\"1\"><sign>G</sign><line>2</line></clef>";
        CHECK( check_result(source, expected) );

        ++it;   //clef F4
        source = exporter.get_source(*it);
        expected = "<clef number=\"2\"><sign>F</sign><line>4</line></clef>";
        CHECK( check_result(source, expected) );
    }

    //@ direction -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, direction_01)
    {
        //@01. direction: coda, segno

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<direction placement=\"above\">"
            "<direction-type>"
                "<coda/>"
            "</direction-type>"
            "</direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction placement=\"above\"><direction-type><coda/>"
                "</direction-type></direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_02)
    {
        //@02. direction. metronome: note-value

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(metronome e. 80)(n d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //metronome

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
            string expected = "<direction><direction-type>"
                "<metronome><beat-unit>eighth</beat-unit>"
                "<beat-unit-dot/>"
                "<per-minute>80</per-minute></metronome>"
                "</direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_03)
    {
        //@03. direction. metronome: note-note

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(metronome e. q)(n d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //metronome

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
            string expected = "<direction><direction-type>"
                "<metronome><beat-unit>eighth</beat-unit>"
                "<beat-unit-dot/>"
                "<beat-unit>quarter</beat-unit></metronome>"
                "</direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_04)
    {
        //@04. direction. metronome: value

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(metronome 70)(n d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //metronome

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
            string expected = "<direction><direction-type>"
                "<metronome><per-minute>70</per-minute></metronome>"
                "</direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_05)
    {
        //@05. direction. words

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(spacer 0 (text \"Largo\"))(n d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //metronome

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
            string expected = "<direction><direction-type>"
                "<words>Largo</words>"
                "</direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_06)
    {
        //@06. direction: dynamics

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<direction placement=\"above\">"
            "<direction-type>"
                "<dynamics><fp/></dynamics>"
            "</direction-type>"
            "</direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction placement=\"above\"><direction-type>"
                "<dynamics><fp/></dynamics></direction-type></direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_07)
    {
        //@07. direction: wedge

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction>"
                "<direction-type><wedge type='crescendo'/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type='stop'/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef (implicit)
        ++it;   //wedge crescendo

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction><direction-type>"
            "<wedge type=\"crescendo\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note
        ++it;   //wedge stop
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<wedge type=\"stop\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

//    TEST_FIXTURE(MxlExporterTestFixture, direction_08)
//    {
//        //@08. direction: pedal line
//
//        Document doc(m_libraryScope);
//        doc.from_string("<score-partwise version='3.0'><part-list>"
//            "<score-part id='P1'><part-name>Music</part-name></score-part>"
//            "</part-list><part id='P1'>"
//            "<measure number='1'>"
//            "<direction><direction-type>"
//                "<pedal type='start' line='yes' sign='yes'/>"
//            "</direction-type></direction>"
//            "<note><pitch><step>G</step><octave>5</octave></pitch>"
//                "<duration>4</duration><type>16th</type>"
//            "</note>"
//            "<direction><direction-type>"
//                "<pedal type='stop' line='yes' sign='yes'/>"
//            "</direction-type></direction>"
//            "</measure>"
//            "</part></score-partwise>", Document::k_format_mxl );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//        ImoObj::children_iterator it = pMD->begin();    //clef (implicit)
//        ++it;   //pedal line start
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        exporter.set_current_instrument(pInstr);
//        string source = exporter.get_source(*it);
//        string expected = "<direction><direction-type>"
//            "<pedal type='start' line='yes' sign='yes'/></direction-type></direction>";
//        CHECK( check_result(source, expected) );
//
//        ++it;   //note
//        ++it;   //pedal line stop
//        source = exporter.get_source(*it);
//        expected = "<direction><direction-type>"
//            "<pedal type='stop' line='yes' sign='yes'/></direction-type></direction>";
//        CHECK( check_result(source, expected) );
//    }

    //@ key -----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, key_00)
    {
        //@00 key. minimal test

        Document doc(m_libraryScope);
        ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                        ImFactory::inject(k_imo_key_signature, &doc));
        pImo->set_key_type(k_key_A);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        CHECK( check_result(source, "<key><fifths>3</fifths><mode>major</mode></key>") );
        delete pImo;
    }

//
//    //@ Lyric ---------------------------------------------------------------------------
//
//    TEST_FIXTURE(MxlExporterTestFixture, lyric_0)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)"
//            "(instrument (musicData (clef G)"
//            "(n c4 q (lyric 1 \"This\")(lyric 2 \"A\"))"
//            "(n d4 q (lyric 1 \"is\")(lyric 2 \"se\" -))"
//            "(n e4 q (lyric 1 \"line\")(lyric 2 \"cond\"))"
//            "(n f4 q (lyric 1 \"one.\")(lyric 2 \"line.\"))"
//            "(barline))))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
////        dump_colection(pScore);
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_current_score(pScore);
//        exporter.set_remove_newlines(true);
//        string source = exporter.get_source(pMD);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(musicData (clef G p1)"
//            "(n c4 q v1 p1 (lyric 1 \"This\" below)(lyric 2 \"A\" below))"
//            "(n d4 q v1 p1 (lyric 1 \"is\" below)(lyric 2 \"se\" - below))"
//            "(n e4 q v1 p1 (lyric 1 \"line\" below)(lyric 2 \"cond\" below))"
//            "(n f4 q v1 p1 (lyric 1 \"one.\" below)(lyric 2 \"line.\" below))"
//            "(barline simple))" );
//    }

    //@ notations -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, notations_01)
    {
        //@01. notations. fermata

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)"
            "(n e4 q (fermata above))"
            "(r q (fermata below))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //note e4

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>E</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><fermata/></notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //rest
        source = exporter.get_source(*it);
        expected = "<note><rest/>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><fermata type=\"inverted\"/></notations></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, notations_02)
    {
        //@02. notations. dynamics

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)"
            "(n e4  q (dyn \"fff\" below))"
            "(n f4  q (dyn \"ppp\"))"
            "(n g4  q (dyn \"sfz\" above))"
            "(n a4  q (dyn \"sfz\" above (dx 50)(dy -70)(color #ff0000)))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //note e4

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>E</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><dynamics placement=\"below\"><fff/></dynamics></notations>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //note f4
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>F</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><dynamics><ppp/></dynamics></notations>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //note g4
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>G</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><dynamics placement=\"above\"><sfz/></dynamics></notations>"
            "</note>";
        CHECK( check_result(source, expected) );
    }

//    //@ dynamics marks -------------------------------------------------------------
//
//    TEST_FIXTURE(MxlExporterTestFixture, dyn_01)
//    {
//        //@01. dynamics marks
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)(instrument (musicData "
//            "(clef G)(key C)"
//            "(n g4  q (dyn \"fff\" below))"
//            "(n g4  q (dyn \"ppp\"))"
//            "(n g4  q (dyn \"sfz\" above))"
//            "(n g4  q (dyn \"sfz\" above (dx 50)(dy -70)(color #ff0000)))"
//            ")))");
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_current_score(pScore);
//        exporter.set_remove_newlines(true);
//        string source = exporter.get_source(pScore);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        string expected = "(score (vers 2.0)(instrument P1 (staves 1)(musicData "
//            "(clef G p1)(key C)"
//            "(n g4 q v1 p1 (dyn \"fff\" below))"
//            "(n g4 q v1 p1 (dyn \"ppp\"))"
//            "(n g4 q v1 p1 (dyn \"sfz\" above))"
//            "(n g4 q v1 p1 (dyn \"sfz\" above (color #ff0000ff)(dx 50)(dy -70)))"
//            ")))";
//        CHECK( source == expected );
//    }

    //@ note ----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, note_00)
    {
        //@00. note without accidentals

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(n d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pImo = static_cast<ImoNote*>(
                                        pMD->get_child_of_type(k_imo_note_regular) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<note><pitch><step>D</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type></note>";
        CHECK( exporter.get_divisions() == 480 );
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_01)
    {
        //@01. note with two dots

        Document doc(m_libraryScope);
        ImoNote* pImo = ImFactory::inject_note(&doc, k_step_B, k_octave_7,
                            k_whole, k_no_accidentals, 2);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<note><pitch><step>B</step><octave>7</octave></pitch>"
            "<duration>3360</duration><voice>0</voice><type>whole</type><dot/><dot/></note>";
        CHECK( check_result(source, expected) );
        delete pImo;
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_02)
    {
        //@02. staff added when instrument has more than one

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (staves 2)"
                "(musicData (clef G p1)(clef F4 p2)(n d4 q p2) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pImo = static_cast<ImoNote*>(
                                        pMD->get_child_of_type(k_imo_note_regular) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pImo);
        string expected = "<note><pitch><step>D</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<staff>2</staff></note>";
        CHECK( exporter.get_divisions() == 480 );
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_03)
    {
        //@03. note with accidentals

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(n +d4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pImo = static_cast<ImoNote*>(
                                        pMD->get_child_of_type(k_imo_note_regular) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pImo);
        string expected = "<note><pitch><step>D</step><alter>1</alter><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<accidental>sharp</accidental></note>";
        CHECK( check_result(source, expected) );
    }

    //@ ornaments -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, ornaments_01)
    {
        //@01. notations. ornaments

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<ornaments><turn placement=\"above\"/></ornaments>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<notations><ornaments><turn placement=\"above\"/></ornaments>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    //@ part-list -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, part_list_00)
    {
        //@00. part-list. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(r q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part-list><score-part id=\"P1\"><part-name/></score-part></part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, part_list_01)
    {
        //@01. part-list. several instruments and part name

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (name \"Voice\")(musicData (clef G)(r q)))"
            "(instrument (name \"Guitar\")(musicData (clef F4)(r q)))"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part-list><score-part id=\"P1\"><part-name>Voice</part-name></score-part>"
            "<score-part id=\"P2\"><part-name>Guitar</part-name></score-part></part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, part_list_02)
    {
        //@02. part-list. group of instruments and part-abbreviation

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(parts"
                "(instrIds S1 T1 P1)"
                "(group S1 T1 (symbol bracket)(joinBarlines yes))"
            ")"
            "(instrument S1 (name \"Soprano\")(abbrev \"S\")(musicData))"
            "(instrument T1 (name \"Tenor\")(abbrev \"T\")(musicData))"
            "(instrument P1 (name \"Piano\")(abbrev \"P\")(staves 2)(musicData))"
            ")");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected =
            "<part-list>"
            "<part-group number=\"1\" type=\"start\">"
                "<group-symbol>bracket</group-symbol>"
                "<group-barline>yes</group-barline>"
            "</part-group>"
            "<score-part id=\"S1\"><part-name>Soprano</part-name><part-abbreviation>S</part-abbreviation></score-part>"
            "<score-part id=\"T1\"><part-name>Tenor</part-name><part-abbreviation>T</part-abbreviation></score-part>"
            "<part-group number=\"1\" type=\"stop\"/>"
            "<score-part id=\"P1\"><part-name>Piano</part-name><part-abbreviation>P</part-abbreviation></score-part>"
            "</part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    //@ rest ----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, rest_00)
    {
        //@00. rest. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(r q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoRest* pImo = static_cast<ImoRest*>(
                                        pMD->get_child_of_type(k_imo_rest) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<note><rest/>"
            "<duration>480</duration><voice>1</voice><type>quarter</type></note>";
        CHECK( exporter.get_divisions() == 480 );
        CHECK( check_result(source, expected) );
    }

    //@ slur ----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, slur_01)
    {
        //@01. slur. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 q (slur 1 start))"
            "(n e4 q (slur 1 stop)) )))"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions><clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><slur number=\"1\" type=\"start\"/>"
            "</notations></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch><duration>480</duration>"
            "<voice>1</voice><type>quarter</type>"
            "<notations><slur number=\"1\" type=\"stop\"/>"
            "</notations></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, slur_02)
    {
        //@02. several slurs

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/003-slur.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
            "<key><fifths>0</fifths><mode>major</mode></key>"
            "<time><beats>3</beats><beat-type>4</beat-type></time>"
            "<staves>2</staves>"
            "<clef number=\"1\"><sign>G</sign><line>2</line></clef>"
            "<clef number=\"2\"><sign>F</sign><line>4</line></clef>"
            "</attributes>"
            "<note><pitch><step>F</step><octave>4</octave></pitch><duration>1440</duration>"
                "<voice>1</voice><type>half</type><dot/><stem>up</stem><staff>1</staff>"
                "<notations><slur number=\"1\" type=\"start\"/></notations></note>"
            "<backup><duration>1440</duration></backup>"
            "<note><pitch><step>A</step><octave>2</octave></pitch><duration>1440</duration>"
                "<voice>2</voice><type>half</type><dot/><stem>up</stem><staff>2</staff>"
                "<notations><slur number=\"2\" type=\"start\"/></notations></note>"
            "</measure>"
            "<measure number=\"2\">"
            "<note><pitch><step>C</step><octave>5</octave></pitch><duration>480</duration>"
                "<voice>1</voice><type>quarter</type><stem>down</stem><staff>1</staff>"
                "<notations><slur number=\"1\" type=\"stop\"/></notations></note>"
            "<note><pitch><step>B</step><octave>4</octave></pitch><duration>960</duration>"
                "<voice>1</voice><type>half</type><stem>up</stem><staff>1</staff>"
                "<notations><slur number=\"1\" type=\"start\"/></notations></note>"
            "<backup><duration>1440</duration></backup>"
            "<note><pitch><step>B</step><octave>1</octave></pitch><duration>480</duration>"
                "<voice>2</voice><type>quarter</type><stem>up</stem><staff>2</staff>"
                "<notations><slur number=\"2\" type=\"stop\"/></notations></note>"
            "<note><pitch><step>D</step><octave>3</octave></pitch><duration>960</duration>"
                "<voice>2</voice><type>half</type><stem>down</stem><staff>2</staff>"
                "<notations><slur number=\"2\" type=\"start\"/></notations></note>"
            "</measure>"
            "<measure number=\"3\">"
            "<note><pitch><step>B</step><octave>4</octave></pitch><duration>480</duration>"
                "<voice>1</voice><type>quarter</type><stem>up</stem><staff>1</staff>"
                "<notations><slur number=\"1\" type=\"stop\"/></notations></note>"
            "<note><rest/><duration>960</duration><voice>1</voice><type>half</type><staff>1</staff></note>"
            "<backup><duration>1440</duration></backup>"
            "<note><pitch><step>C</step><octave>3</octave></pitch><duration>480</duration>"
                "<voice>2</voice><type>quarter</type><stem>up</stem><staff>2</staff>"
                "<notations><slur number=\"2\" type=\"stop\"/></notations></note>"
            "<note><rest/><duration>960</duration><voice>2</voice><type>half</type><staff>2</staff></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    //@ technical -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, technical_01)
    {
        //@01. technical notations. fingering

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><fingering>3</fingering></technical>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<notations><technical><fingering>3</fingering></technical>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, technical_02)
    {
        //@02. technical notations. fret-string

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><fret>3</fret><string>4</string></technical>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<notations><technical><fret>3</fret><string>4</string></technical>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, technical_03)
    {
        //@03. technical notations. other

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><up-bow placement=\"above\"/></technical>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( errormsg.str().empty() );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<notations><technical><up-bow placement=\"above\"/></technical>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    //@ tie -----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, tie_01)
    {
        //@01. tie. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 q (tie 1 start))"
            "(n c4 q (tie 1 stop)) )))"
            );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //first note

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><tie type=\"start\"/><voice>1</voice><type>quarter</type>"
            "<notations><tied type=\"start\"/></notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //second note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><tie type=\"stop\"/><voice>1</voice><type>quarter</type>"
            "<notations><tied type=\"stop\"/></notations></note>";
        CHECK( check_result(source, expected) );
    }

    //@ time signature ------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, time_signature_0)
    {
        //@00. minimal test. beats and beat type

        Document doc(m_libraryScope);
        ImoTimeSignature* pImo =
            static_cast<ImoTimeSignature*>(ImFactory::inject(k_imo_time_signature, &doc));
        pImo->set_top_number(6);
        pImo->set_bottom_number(8);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        CHECK( check_result(source, "<time><beats>6</beats><beat-type>8</beat-type></time>") );
        delete pImo;
    }

//    TEST_FIXTURE(MxlExporterTestFixture, time_signature_0)
//    {
//        //@00. time, type
//
//        Document doc(m_libraryScope);
//        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
//                            ImFactory::inject(k_imo_time_signature, &doc));
//        pImo->set_type(ImoTimeSignature::k_single_number);
//        pImo->set_top_number(10);
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        string source = exporter.get_source(pImo);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(time single-number 10)" );
//        delete pImo;
//    }

//    TEST_FIXTURE(MxlExporterTestFixture, time_signature_1)
//    {
//        //@01. time id
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0) (instrument#100"
//                "(musicData (time#123 5 4) )))");
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
//                                        pMD->get_child_of_type(k_imo_time_signature) );
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        exporter.set_add_id(true);
//        string source = exporter.get_source(pImo);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(time#123 5 4)" );
//        CHECK( check_result(source, "<key><fifths>3</fifths><mode>major</mode></key>") );
//    }

//    TEST_FIXTURE(MxlExporterTestFixture, time_signature_2)
//    {
//        //@02. time. Attachments
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0) (instrument#100"
//                "(musicData (time common (text \"Hello\")) )))");
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//        ImoTimeSignature* pImo = static_cast<ImoTimeSignature*>(
//                                        pMD->get_child_of_type(k_imo_time_signature) );
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        string source = exporter.get_source(pImo);
//        //cout << test_name() << endl << "\"" << source << "\"" << endl;
//        CHECK( source == "(time common (text \"Hello\"))" );
//    }

    //@ tuplet --------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, tuplet_01)
    {
        //@01. tuplet. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(n c4 e t3)(n e4 e)(n g4 e t-)"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //first note

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "<notations><tuplet type=\"start\" number=\"1\" show-number=\"actual\">"
            "<tuplet-actual><tuplet-number>3</tuplet-number></tuplet-actual>"
            "<tuplet-normal><tuplet-number>2</tuplet-number></tuplet-normal></tuplet>"
            "</notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //second note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>E</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //third note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>G</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "<notations><tuplet type=\"stop\" number=\"1\"/></notations>"
            "</note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, tuplet_02)
    {
        //@02. nested tuplets

        Document doc(m_libraryScope);
        //this score is a copy from 01014-nested-tuplets.lms and it is also equal
        //to Lilypond 23d-Tuplets-Nested.xml
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)(time 2 4)"
            "(n b4 e (tm 2 3)(beam 1 +)(t 1 + 3 2))(n b4 e (tm 2 3)(beam 1 -))"
            "(n b4 e (tm 4 15)(beam 2 +)(t 2 + 5 2 (displayBracket yes)))"
            "(n b4 e (tm 4 15)(beam 2 =))(n b4 e (tm 4 15)(beam 2 =))"
            "(n b4 e (tm 4 15)(beam 2 =))(n b4 e (tm 4 15)(beam 2 -)(t 2 -))"
            "(n b4 e (tm 2 3)(beam 3 +))(n b4 e (tm 2 3)(beam 3 -)(t 1 -))"
            ")))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G
        ++it;   //time signature
        ++it;   //first note

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "<beam number=\"1\">begin</beam>"
            "<notations><tuplet type=\"start\" number=\"1\" show-number=\"actual\">"
            "<tuplet-actual><tuplet-number>3</tuplet-number></tuplet-actual>"
            "<tuplet-normal><tuplet-number>2</tuplet-number></tuplet-normal></tuplet>"
            "</notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //second note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "<beam number=\"1\">end</beam>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //3rd note: start of second tuplet (nested)
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>64</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>15</actual-notes><normal-notes>4</normal-notes></time-modification>"
            "<beam number=\"1\">begin</beam>"
            "<notations><tuplet type=\"start\" number=\"2\" bracket=\"yes\" show-number=\"actual\">"
            "<tuplet-actual><tuplet-number>5</tuplet-number></tuplet-actual>"
            "<tuplet-normal><tuplet-number>2</tuplet-number></tuplet-normal></tuplet>"
            "</notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //4th note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>64</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>15</actual-notes><normal-notes>4</normal-notes></time-modification>"
            "<beam number=\"1\">continue</beam>"
            "</note>";
        CHECK( check_result(source, expected) );

        ++it;   //5th note
        ++it;   //6th note
        ++it;   //7th note: end of second tuplet
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>64</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>15</actual-notes><normal-notes>4</normal-notes></time-modification>"
            "<beam number=\"1\">end</beam>"
            "<notations><tuplet type=\"stop\" number=\"2\"/>"
            "</notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //8th note
        ++it;   //9th note: end of first tuplet
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>B</step><octave>4</octave></pitch>"
            "<duration>160</duration><voice>1</voice><type>eighth</type>"
            "<time-modification><actual-notes>3</actual-notes><normal-notes>2</normal-notes></time-modification>"
            "<beam number=\"1\">end</beam>"
            "<notations><tuplet type=\"stop\" number=\"1\"/>"
            "</notations></note>";
        CHECK( check_result(source, expected) );
    }
////
////
////    // color ------------------------------------------------------------------------------------
////
////    TEST_FIXTURE(MxlExporterTestFixture, color)
////    {
////        Document doc(m_libraryScope);
////        ImoClef* pImo = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
////        pImo->set_clef_type(k_clef_F4);
////        pImo->set_color( Color(127, 40, 12, 128) );
////        MxlExporter exporter(m_libraryScope);
////        string source = exporter.get_source(pImo);
//////        cout << test_name() << endl << "\"" << source << "\"" << endl;
////        CHECK( source == "(clef F4 p1 (color #7f280c80))" );
////        delete pImo;
////    }
////
//////    // user location ----------------------------------------------------------------------------
//////
//////    TEST_FIXTURE(MxlExporterTestFixture, ExportMxl_user_location)
//////    {
//////        ImoClef obj;
//////        obj.set_type(k_clef_G2);
//////        obj.set_user_location_x(30.0f);
//////        obj.set_user_location_y(-7.05f);
//////        MxlExporter exporter(m_libraryScope);
//////        string source = exporter.get_source(&obj);
//////        cout << test_name() << endl << "\"" << source << "\"" << endl;
//////        CHECK( source == "(clef G3 p1 (dx 30.0000) (dy -7.0500))" );
//////    }

};
