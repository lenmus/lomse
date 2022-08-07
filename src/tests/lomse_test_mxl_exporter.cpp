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

    inline void failure_header()
    {
        cout << endl << "*** Failure in " << test_name() << ":" << endl;
    }

    bool check_result(const string& ss, const string& expected)
    {
        if (ss != expected)
        {
            failure_header();
            cout << "  result=[" << ss << "]" << endl;
            cout << endl << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_result_contains(const string& ss, const string& expected, const string& tag)
    {
        if (ss.find(expected) == std::string::npos)
        {
            failure_header();
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
            cout << endl << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_result_contains(const string& ss, const string& expected)
    {
        if (ss.find(expected) == std::string::npos)
        {
            failure_header();
            cout << "  result=[" << ss << "]" << endl;
            cout << endl << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_result_contains(const string& ss, const string& expected,
                               const string& start, const string& stop)
    {
        if (ss.find(expected) == std::string::npos)
        {
            failure_header();
            size_t iStart = ss.find(start);
            if(iStart == std::string::npos)
                cout << "  result=[" << ss << "]" << endl;
            else
            {
                size_t iEnd = ss.find(stop, iStart);
                if(iEnd == std::string::npos)
                    cout << "  result=[" << ss.substr(iStart) << "]" << endl;
                else
                    cout << "  result=[" << ss.substr(iStart, iEnd-iStart) << "]" << endl;
            }
            cout << endl << "expected=[" << expected << "]" << endl;
            return false;
        }
        return true;
    }

    bool check_errormsg_empty(const string& msg)
    {
        if (!msg.empty())
        {
            failure_header();
            cout << "Expected empty error msg. but msg=[" << msg << "]" << endl;
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
//        //doc.from_file(m_scores_path + "00205-multimetric.lmd", Document::k_format_lmd );
//        //doc.from_file(m_scores_path + "00023-spacing-in-prolog-two-instr.lms" );
////        doc.from_file(m_scores_path + "50120-fermatas.lms" );
//        //doc.from_file(m_scores_path + "unit-tests/xml-export/023-grace-direction-at-start.xml", Document::k_format_mxl);
//        //doc.from_file(m_scores_path + + "unit-tests/colstaffobjs/09-cross-staff-beamed-group-with-intermediate-clef.lms");
//        //doc.from_file("/datos/cecilio/lm/projects/lomse/vregress/scores/lilypond/43d-MultiStaff-StaffChange.xml", Document::k_format_mxl);
//        doc.from_file("/datos/cecilio/lm/projects/lomse/vregress/scores/recordare/DebuMandSample.musicxml", Document::k_format_mxl);
////        doc.from_file(m_scores_path + "00110-triplet-against-5-tuplet-4.14.lms" );
////        doc.from_file(m_scores_path + "50130-metronome.lms" );
////        doc.from_file(m_scores_path + "50180-new-system-tag.lms" );
////        doc.from_file(m_scores_path + "50110-graphic-line-text.lms" );
////        doc.from_file("/datos/cecilio/Desarrollo_wx/lomse/samples/chopin_prelude20_v16.lms" );
//        ImoDocument* pRoot = doc.get_im_root();
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_add_id(true);
//
//        ofstream file1(m_scores_path + "../z_test_musicxml_export.xml", ios::out);
//        if (file1.good())
//        {
//            string source = exporter.get_source(pRoot);
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

    TEST_FIXTURE(MxlExporterTestFixture, attributes_02)
    {
        //@02. attributes: staff-details

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<clef><sign>G</sign><line>2</line></clef>"
                "<staff-details>"
                    "<staff-lines>1</staff-lines>"
                    "<staff-size scaling=\"100\">167</staff-size>"
                "</staff-details>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
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
                "<staff-details>"
                    "<staff-lines>1</staff-lines>"
                    "<staff-size scaling=\"100\">167</staff-size>"
                "</staff-details>";
        CHECK( check_result_contains(source, expected, "staff-details") );
    }

    //@ backup --------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, backup_01)
    {
        //@01. backup. two voices.

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_01)
    {
        //@01. barline. end barline

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_02)
    {
        //@02. barline. regular barline

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_03)
    {
        //@03. barline. start repetition

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_04)
    {
        //@04. barline. double repetition

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_05)
    {
        //@05. barline. no visible

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

    TEST_FIXTURE(MxlExporterTestFixture, barline_06)
    {
        //@06. barline. end barline. multi-instrument

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/014-end-barlines.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_remove_separator_lines(true);

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>D</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );

        pInstr = pScore->get_instrument(1);
        pMD = pInstr->get_musicdata();
        exporter.set_current_instrument(pInstr);
        source = exporter.get_source(pMD);
        expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
                "<clef><sign>F</sign><line>4</line></clef>"
            "</attributes>"
            "<note><pitch><step>A</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-heavy</bar-style></barline>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_07)
    {
        //@07. barline. volta-bracket

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/015-volta-brackets.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);

        string source = exporter.get_source(pMD);
        string expected = "<measure number=\"2\">"
            "<barline location=\"left\"><ending number=\"1\" type=\"start\"/></barline>";
        CHECK( check_result_contains(source, expected, "<measure number=\"2\">", "</barline>") );

        expected = "<barline>"
            "<bar-style>light-heavy</bar-style><ending number=\"1\" type=\"stop\"/>"
            "<repeat direction=\"backward\"/></barline>";
        CHECK( check_result_contains(source, expected, "<barline>", "</barline>") );

        expected = "<measure number=\"3\">"
            "<barline location=\"left\"><ending number=\"2\" type=\"start\"/></barline>";
        CHECK( check_result_contains(source, expected, "<measure number=\"3\">", "</barline>") );

        expected = "<barline>"
            "<ending number=\"2\" type=\"discontinue\"/></barline>";
        CHECK( check_result_contains(source, expected, "<barline><ending number=\"2\"", "</barline>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_08)
    {
        //@08. barline. repeat times

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
                "<attributes>"
                    "<divisions>4</divisions>"
                    "<clef><sign>G</sign><line>2</line></clef>"
                "</attributes>"
                "<note><rest/><duration>16</duration><type>whole</type></note>"
            "</measure>"
            "<measure number='2'>"
                "<barline location='left'>"
                    "<bar-style>heavy-light</bar-style>"
                    "<repeat direction='forward'/>"
                "</barline>"
                "<note><pitch><step>A</step><octave>3</octave></pitch>"
                    "<duration>16</duration><type>whole</type></note>"
                "<barline location='right'>"
                    "<bar-style>light-heavy</bar-style>"
                    "<repeat direction='backward' times='3'/>"
                "</barline>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<measure number=\"2\">"
            "<barline location=\"left\"><bar-style>heavy-light</bar-style>"
            "<repeat direction=\"forward\"/></barline>";
        CHECK( check_result_contains(source, expected, "<measure number=\"2\"", "</barline>") );

        expected = "<barline><bar-style>light-heavy</bar-style>"
            "<repeat direction=\"backward\" times=\"3\"/></barline>";
        CHECK( check_result_contains(source, expected, "<barline location=\"right", "</barline>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, barline_09)
    {
        //@09. barline. double repetition, heavy-heavy

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/029-barline-double-repetition-heavy.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);

        string source = exporter.get_source(pMD);
        string expected = ""
            "<barline><bar-style>heavy-heavy</bar-style>"
            "<repeat direction=\"backward\"/></barline>"
            "</measure><measure implicit=\"yes\" number=\"X1\">"
            "<barline location=\"left\"><repeat direction=\"forward\"/>"
            "</barline>";
        CHECK( check_result_contains(source, expected, "<barline>", "<note>") );
    }

    // @ beam ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, beam_01)
    {
        //@01. beam. begin and end

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

    TEST_FIXTURE(MxlExporterTestFixture, beam_02)
    {
        //@02. beam. chord: beam only in base note. backward hook

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

    TEST_FIXTURE(MxlExporterTestFixture, clef_01)
    {
        //@01 clef. minimal test. sign and line

        Document doc(m_libraryScope);
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_F4);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pClef);
        string expected = "<attributes><clef><sign>F</sign><line>4</line></clef>";
        CHECK( check_result(source, expected) );
        delete pClef;
    }

    TEST_FIXTURE(MxlExporterTestFixture, clef_02)
    {
        //@02. clef. staff added when instrument has more than one

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
        string expected = "<attributes><clef number=\"1\"><sign>G</sign><line>2</line></clef>";
        CHECK( check_result(source, expected) );

        ++it;   //clef F4
        source = exporter.get_source(*it);
        expected = "<clef number=\"2\"><sign>F</sign><line>4</line></clef>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, clef_03)
    {
        //@03. clef. intermediate clef

        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0) (instrument (musicData"
//                "(clef G)(n c4 q)(clef F4)(n d3 q) )))");
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<attributes><clef><sign>F</sign><line>4</line></clef></attributes>"
            "<note><pitch><step>D</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

//        dump_colection(pScore);

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected =
            "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<attributes><clef><sign>F</sign><line>4</line></clef></attributes>"
            "<note><pitch><step>D</step><octave>3</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, clef_04)
    {
        //@04. clef. print-style, print-object

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData"
                "(clef G (visible no)(dx 10)) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<attributes><clef relative-x=\"10\" print-object=\"no\">"
            "<sign>G</sign><line>2</line></clef>";
        CHECK( check_result(source, expected) );
    }

    //@ defaults ------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, defaults_01)
    {
        //@01. defaults not exported when values not modified (imported or user set)

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData"
                "(clef G)(n c4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "</identification><part-list>";
        CHECK( check_result_contains(source, expected, "</identification>", "<score-part") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_02)
    {
        //@02. defaults. scaling

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<scaling><millimeters>7.2</millimeters><tenths>40</tenths></scaling>"
                "</defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults><scaling><millimeters>7.2</millimeters>"
            "<tenths>40</tenths></scaling></defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_03)
    {
        //@03. defaults. page layout

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<scaling><millimeters>3.7703</millimeters><tenths>40</tenths></scaling>"
            "<page-layout>"
                "<page-height>954</page-height><page-width>1804</page-width>"
                "<page-margins type=\"both\">"
                "<left-margin>318</left-margin><right-margin>212</right-margin>"
                "<top-margin>53</top-margin><bottom-margin>74</bottom-margin>"
                "</page-margins></page-layout></defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected =
            "<scaling><millimeters>3.7703</millimeters><tenths>40</tenths></scaling>"
            "<page-layout>"
            "<page-height>954</page-height><page-width>1804</page-width>"
            "<page-margins type=\"both\">"
            "<left-margin>318</left-margin><right-margin>212</right-margin>"
            "<top-margin>53</top-margin><bottom-margin>74</bottom-margin>"
            "</page-margins></page-layout>";
        CHECK( check_result_contains(source, expected, "<scaling>", "</page-layout>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_04)
    {
        //@04. defaults. system layout

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<scaling><millimeters>3.7703</millimeters><tenths>40</tenths></scaling>"
            "<system-layout>"
                "<system-margins><left-margin>248</left-margin>"
                    "<right-margin>206</right-margin></system-margins>"
                "<system-distance>561</system-distance>"
                "<top-system-distance>436</top-system-distance>"
            "</system-layout></defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults>"
            "<scaling><millimeters>3.7703</millimeters><tenths>40</tenths></scaling>"
            "<system-layout><system-margins><left-margin>248</left-margin>"
                "<right-margin>206</right-margin></system-margins>"
            "<system-distance>561</system-distance>"
            "<top-system-distance>436</top-system-distance>"
            "</system-layout></defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
    }

//    TEST_FIXTURE(MxlExporterTestFixture, defaults_05)
//    {
//        //@05. defaults. staff layout
//
//        Document doc(m_libraryScope);
//        doc.from_string("<score-partwise version='3.0'>"
//            "<defaults>"
//            "<scaling><millimeters>3.7703</millimeters><tenths>40</tenths></scaling>"
//            "<staff-layout>"
//                "<staff-distance>90</staff-distance>"
//            "</staff-layout></defaults>"
//            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
//            "</part-list><part id='P1'>"
//            "<measure number='1'>"
//            "</measure>"
//            "</part></score-partwise>", Document::k_format_mxl );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        string source = exporter.get_source(pScore);
//        string expected = "<staff-layout>"
//                "<staff-distance>90</staff-distance></staff-layout>";
//        CHECK( check_result_contains(source, expected, "staff-layout") );
//    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_06)
    {
        //@06. defaults: word-font

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<word-font font-family=\"Times New Roman\" font-size=\"10.2\"/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults>"
            "<word-font font-family=\"Times New Roman\" font-size=\"10.2\"/>"
            "</defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_07)
    {
        //@07. defaults: lyric-font. No: number, language

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font font-family=\"Times New Roman\" font-size=\"10\"/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font font-family=\"Times New Roman\" font-size=\"10\"/>"
            "</defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_08)
    {
        //@08. defaults: lyric-font with lyric-language

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font font-family=\"Times New Roman\" font-size=\"10\"/>"
            "<lyric-language xml:lang=\"fr\"/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font font-family=\"Times New Roman\" font-size=\"10\"/>"
            "<lyric-language xml:lang=\"fr\"/>"
            "</defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, defaults_09)
    {
        //@09. defaults: three lyric-font with language

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'>"
            "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font number=\"1\" font-family=\"Times New Roman\" font-size=\"10.25\"/>"
            "<lyric-font number=\"2\" font-family=\"ＭＳ ゴシック\" font-size=\"10.25\"/>"
            "<lyric-font number=\"3\" font-family=\"Georgia\" font-size=\"10.25\"/>"
            "<lyric-language number=\"1\" xml:lang=\"fr\"/>"
            "<lyric-language number=\"2\" xml:lang=\"ja\"/>"
            "<lyric-language number=\"3\" xml:lang=\"es\"/>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<defaults>"
            "<word-font font-family=\"Sans serif\" font-size=\"10.2\"/>"
            "<lyric-font number=\"1\" font-family=\"Times New Roman\" font-size=\"10.25\"/>"
            "<lyric-font number=\"2\" font-family=\"ＭＳ ゴシック\" font-size=\"10.25\"/>"
            "<lyric-font number=\"3\" font-family=\"Georgia\" font-size=\"10.25\"/>"
            "<lyric-language number=\"1\" xml:lang=\"fr\"/>"
            "<lyric-language number=\"2\" xml:lang=\"ja\"/>"
            "<lyric-language number=\"3\" xml:lang=\"es\"/>"
            "</defaults>";
        CHECK( check_result_contains(source, expected, "defaults") );
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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
                "(musicData (clef G)(metronome l. q)(n d4 q) )))");
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
                "<metronome><beat-unit>long</beat-unit>"
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

    TEST_FIXTURE(MxlExporterTestFixture, direction_08)
    {
        //@08. direction: octave-shift

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/005-octave-shift.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //rest with attached octave-shif start

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction><direction-type>"
            "<octave-shift type=\"down\" size=\"8\" number=\"1\"/>"
            "</direction-type></direction>"
            "<note><rest/><duration>480</duration><voice>1</voice><type>quarter</type></note>";
        CHECK( check_result(source, expected) );

        ++it;   //note F5
        ++it;   //note E5
        ++it;   //note D5 with attached octave-shif stop
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>D</step><octave>5</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type><stem>up</stem></note>"
            "<direction><direction-type>"
            "<octave-shift type=\"stop\" size=\"8\" number=\"1\"/>"
            "</direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_09)
    {
        //@09. direction: pedal mark

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<direction placement=\"below\"><direction-type>"
                "<pedal line=\"no\" type=\"start\"/>"
            "</direction-type></direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction placement=\"below\"><direction-type>"
            "<pedal type=\"start\" line=\"no\"/>"
            "</direction-type></direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_10)
    {
        //@10. direction: pedal lines: types start & stop

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/008-pedal-line.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //pedal line start

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction><direction-type>"
            "<pedal type=\"start\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note
        ++it;   //pedal line stop
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"stop\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_11)
    {
        //@11. direction: pedal marks with line

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/009-pedal-line-marks.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //pedal line start

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction><direction-type>"
            "<pedal type=\"start\" line=\"yes\" sign=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note
        ++it;   //pedal line stop
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"stop\" line=\"yes\" sign=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_12)
    {
        //@12. direction: pedal lines: types change, discontinue & resume

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/010-pedal-lines.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //key
        ++it;   //time
        ++it;   //pedal line start

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction><direction-type>"
            "<pedal type=\"start\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note 1
        ++it;   //note 2
        ++it;   //note 3
        ++it;   //pedal line change
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"change\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note 4
        ++it;   //note 5
        ++it;   //pedal line discontinue
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"discontinue\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note 6
        ++it;   //barline
        ++it;   //note 7
        ++it;   //note 8
        ++it;   //pedal line resume
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"resume\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note 9
        ++it;   //pedal line change
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"change\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note 10
        ++it;   //note 11
        ++it;   //pedal line stop
        source = exporter.get_source(*it);
        expected = "<direction><direction-type>"
            "<pedal type=\"stop\" line=\"yes\"/></direction-type></direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_13)
    {
        //@13. direction: pedal lines: type sostenuto

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<direction placement=\"below\"><direction-type>"
                "<pedal line=\"no\" type=\"sostenuto\"/>"
            "</direction-type></direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction placement=\"below\"><direction-type>"
            "<pedal type=\"sostenuto\" line=\"no\"/>"
            "</direction-type></direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_14)
    {
        //@14. direction: sound, only attribs

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<direction placement=\"below\">"
            "<direction-type><dynamics><p/></dynamics></direction-type>"
            "<sound dynamics=\"54\"/>"
            "</direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction placement=\"below\">"
                "<direction-type><dynamics><p/></dynamics></direction-type>"
                "<sound dynamics=\"54\"/>"
                "</direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_15)
    {
        //@15. direction: sound, midi content

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/011-sound-midi-info.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef (implicit)
        ++it;   //direction

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction placement=\"above\">"
            "<direction-type><words>pizz.</words></direction-type>"
            "<sound><midi-instrument id=\"P3-I4\"><midi-program>46</midi-program>"
                "</midi-instrument></sound>"
            "</direction>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_16)
    {
        //@16. direction. sound as child of <measure>

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<sound tempo=\"84\"/>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //sound

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<sound tempo=\"84\"/>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_17)
    {
        //@17. direction. dynamics moved to <note> in importer is restored in exporter

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/012-directive-moved-to-note.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef
        ++it;   //direction

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<direction placement=\"below\">"
            "<direction-type><dynamics><p/></dynamics></direction-type>"
            "<sound dynamics=\"54\"/>"
            "</direction>";
        CHECK( check_result(source, expected) );

        ++it;   //note
        source = exporter.get_source(*it);
        expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_18)
    {
        //@18. direction. words placement

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<direction><direction-type>"
                "<words default-x=\"-1\" default-y=\"15\" "
                "font-size=\"medium\" font-weight=\"bold\">Bold, Medium</words>"
            "</direction-type></direction>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_direction() == true );
        ImoDirection* pImo = dynamic_cast<ImoDirection*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<direction><direction-type>"
                "<words default-x=\"-1\" default-y=\"15\">Bold, Medium</words>"
                "</direction-type></direction>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_19)
    {
        //@19. direction after grace note in other staff (bug detected)

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/023-grace-direction-at-start.xml", Document::k_format_mxl);
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
            "<measure number=\"1\"><attributes><divisions>480</divisions><staves>2</staves>"
            "<clef number=\"1\"><sign>G</sign><line>2</line></clef>"
            "<clef number=\"2\"><sign>F</sign><line>4</line></clef></attributes>"
            "<note><grace steal-time-previous=\"20\" slash=\"yes\"/><pitch><step>G</step>"
            "<octave>3</octave></pitch><voice>1</voice><type>eighth</type><stem>up</stem>"
            "<staff>2</staff></note>"
            "<direction placement=\"below\"><direction-type><dynamics><sfp/></dynamics>"
            "</direction-type><staff>1</staff><sound dynamics=\"54\"/></direction>"
            "<note><pitch><step>G</step><octave>4</octave></pitch>"
            "<duration>1440</duration><voice>1</voice><type>half</type><dot/><stem>up</stem>"
            "<staff>1</staff></note>"
            "<backup><duration>1440</duration></backup>"
            "<note><rest><display-step>D</display-step><display-octave>3</display-octave></rest>"
            "<duration>1440</duration><voice>2</voice><type>whole</type><staff>2</staff></note>"
            "</measure>";
        CHECK( check_result(source, expected) );

        expected =
            "<direction placement=\"below\"><direction-type><dynamics><sfp/></dynamics>"
            "</direction-type><staff>1</staff><sound dynamics=\"54\"/></direction>";
        CHECK( check_result_contains(source, expected, "<direction", "<note>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_20)
    {
        //@20. direction. span direction ends before <backup> for next voice (bug detected)

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/024-span-direction-backup.xml", Document::k_format_mxl);
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
            "<direction><direction-type><wedge type=\"stop\"/></direction-type>"
            "<staff>1</staff></direction>";
        CHECK( check_result_contains(source, expected, "<direction>", "<backup>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, direction_21)
    {
        //@21. direction: wedge attributes

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<direction><direction-type>"
                "<wedge default-y=\"-73\" spread=\"11\" type=\"diminuendo\"/></direction-type>"
            "</direction>"
            "<note><pitch><step>G</step><octave>5</octave></pitch>"
                "<duration>4</duration><type>16th</type>"
            "</note>"
            "<direction>"
                "<direction-type><wedge type=\"stop\" niente=\"yes\"/></direction-type>"
            "</direction>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);

        string expected = "<wedge type=\"diminuendo\" spread=\"11\"/>";
        CHECK( check_result_contains(source, expected, "<wedge type=\"diminuendo\"", "</direction-type") );

        expected = "<wedge type=\"stop\" niente=\"yes\"/>";
        CHECK( check_result_contains(source, expected, "<wedge type=\"stop\"", "</direction-type") );
    }


    //@ key -----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, key_01)
    {
        //@01 key. minimal test

        Document doc(m_libraryScope);
        ImoKeySignature* pImo = static_cast<ImoKeySignature*>(
                                        ImFactory::inject(k_imo_key_signature, &doc));
        pImo->set_key_type(k_key_A);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<attributes><key><fifths>3</fifths><mode>major</mode></key>";
        CHECK( check_result(source, expected) );
        delete pImo;
    }

    TEST_FIXTURE(MxlExporterTestFixture, key_02)
    {
        //@02 non-standard key. Several accidentals.

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<key>"
                "<key-step>G</key-step>"
                "<key-alter>-1.5</key-alter>"
                "<key-step>A</key-step>"
                "<key-alter>1.5</key-alter>"
                "<key-step>B</key-step>"
                "<key-alter>-0.5</key-alter>"
            "</key>"
        );
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pKey);
            string expected = "<attributes><key><key-step>G</key-step>"
                "<key-alter>-1.5</key-alter>"
                "<key-accidental>three-quarters-flat</key-accidental>"
                "<key-step>A</key-step><key-alter>1.5</key-alter>"
                "<key-accidental>three-quarters-sharp</key-accidental>"
                "<key-step>B</key-step><key-alter>-0.5</key-alter>"
                "<key-accidental>quarter-flat</key-accidental></key>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    TEST_FIXTURE(MxlExporterTestFixture, key_03)
    {
        //@03. key: number

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/013-key-number.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected =
            "<attributes><divisions>1</divisions>"
                "<key number=\"1\"><fifths>1</fifths><mode>major</mode></key>"
                "<key number=\"2\"><fifths>3</fifths><mode>major</mode></key>"
                "<staves>2</staves>"
                "<clef number=\"1\"><sign>G</sign><line>2</line></clef>"
                "<clef number=\"2\"><sign>F</sign><line>4</line></clef>"
            "</attributes>";
        CHECK( check_result_contains(source, expected, "attributes") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, key_04)
    {
        //@04. key: print-style, print-object

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData"
                "(key A (visible no)(dx 10)) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<attributes><key relative-x=\"10\" print-object=\"no\">"
            "<fifths>3</fifths><mode>major</mode></key>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, key_05)
    {
        //@05 non-standard key. key-octave

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<key>"
                "<key-step>G</key-step>"
                "<key-alter>-1.5</key-alter>"
                "<key-step>A</key-step>"
                "<key-alter>1.5</key-alter>"
                "<key-octave number=\"1\">2</key-octave>"
                "<key-octave number=\"2\">3</key-octave>"
            "</key>"
        );
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_key_signature() == true );
        ImoKeySignature* pKey = dynamic_cast<ImoKeySignature*>( pRoot );
        CHECK( pKey != nullptr );
        if (pKey)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pKey);
            string expected = "<attributes><key><key-step>G</key-step>"
                "<key-alter>-1.5</key-alter>"
                "<key-accidental>three-quarters-flat</key-accidental>"
                "<key-step>A</key-step><key-alter>1.5</key-alter>"
                "<key-accidental>three-quarters-sharp</key-accidental>"
                "<key-octave number=\"1\">2</key-octave>"
                "<key-octave number=\"2\">3</key-octave></key>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

    //@ lyric ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, lyric_01)
    {
        //@01. lyric. minimal test

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric 1 \"This\"))"
            ")))" );
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
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<lyric number=\"1\"><syllabic>single</syllabic><text>This</text></lyric></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, lyric_02)
    {
        //@02. lyric. two lines

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric 1 \"This\")(lyric 2 \"A\"))"
            ")))" );
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
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<lyric number=\"1\"><syllabic>single</syllabic><text>This</text></lyric>"
            "<lyric number=\"2\"><syllabic>single</syllabic><text>A</text></lyric></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, lyric_03)
    {
        //@03. lyric. begin, middle and end. hyphenation

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n d4 q (lyric 1 \"to\" -))"
            "(n e4 q (lyric 1 \"ma\" -))"
            "(n f4 q (lyric 1 \"te.\"))"
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
            "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><pitch><step>D</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<lyric number=\"1\"><syllabic>begin</syllabic><text>to</text></lyric></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<lyric number=\"1\"><syllabic>middle</syllabic><text>ma</text></lyric></note>"
            "<note><pitch><step>F</step><octave>4</octave></pitch>"
                "<duration>480</duration><voice>1</voice><type>quarter</type>"
                "<lyric number=\"1\"><syllabic>end</syllabic><text>te.</text></lyric></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, lyric_04)
    {
        //@04. lyric. melisma line

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric \"Ah\" (melisma)))"
            ")))" );
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
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<lyric number=\"1\"><syllabic>single</syllabic><text>Ah</text>"
            "<extend/></lyric></note>";
        CHECK( check_result(source, expected) );
    }

//    TEST_FIXTURE(MxlExporterTestFixture, lyric_05)
//    {
//        //@05. lyric. implicit end of melisma line. lyric exists
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)"
//            "(instrument (musicData (clef G)"
//            "(n c4 q (lyric \"Ah\" (melisma)))"
//            "(n e4 q (lyric \"Son\"))"
//            ")))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        exporter.set_current_score(pScore);
//        exporter.set_current_instrument(pInstr);
//        exporter.set_remove_separator_lines(true);
//        string source = exporter.get_source(pMD);
//        string expected =
//            "<measure number=\"1\">"
//            "<attributes><divisions>480</divisions>"
//            "<clef><sign>G</sign><line>2</line></clef>"
//            "</attributes>"
//            "<note><pitch><step>C</step><octave>4</octave></pitch>"
//                "<duration>480</duration><voice>1</voice><type>quarter</type>"
//                "<lyric number=\"1\"><syllabic>single</syllabic><text>Ah</text>"
//                "<extend type=\"start\"/></lyric></note>"
//            "<note><pitch><step>E</step><octave>4</octave></pitch>"
//                "<duration>480</duration><voice>1</voice><type>quarter</type>"
//                "<lyric number=\"1\"><syllabic>single</syllabic><text>Son</text>"
//                "<extend type=\"stop\"/></lyric></note>"
//            "</measure>";
//        CHECK( check_result(source, expected) );
//    }

//    TEST_FIXTURE(MxlExporterTestFixture, lyric_06)
//    {
//        //@06. lyric. implicit end of melisma line. no lyric
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0)"
//            "(instrument (musicData (clef G)"
//            "(n c4 q (lyric \"Ah\" (melisma)))"
//            "(n e4 q)"
//            ")))" );
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        ImoInstrument* pInstr = pScore->get_instrument(0);
//        ImoMusicData* pMD = pInstr->get_musicdata();
//
//        MxlExporter exporter(m_libraryScope);
//        exporter.set_remove_newlines(true);
//        exporter.set_current_score(pScore);
//        exporter.set_current_instrument(pInstr);
//        exporter.set_remove_separator_lines(true);
//        string source = exporter.get_source(pMD);
//        string expected =
//            "<measure number=\"1\">"
//            "<attributes><divisions>480</divisions>"
//            "<clef><sign>G</sign><line>2</line></clef>"
//            "</attributes>"
//            "<note><pitch><step>C</step><octave>4</octave></pitch>"
//                "<duration>480</duration><voice>1</voice><type>quarter</type>"
//                "<lyric number=\"1\"><syllabic>single</syllabic><text>Ah</text>"
//                "<extend type=\"start\"/></lyric></note>"
//            "<note><pitch><step>E</step><octave>4</octave></pitch>"
//                "<duration>480</duration><voice>1</voice><type>quarter</type>"
//                "<lyric number=\"1\"><extend type=\"stop\"/></lyric></note>"
//            "</measure>";
//        CHECK( check_result(source, expected) );
//    }

    TEST_FIXTURE(MxlExporterTestFixture, lyric_07)
    {
        //@07. lyric. two syllables. elision

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric 1 \"This\" \"is\"))"
            ")))" );
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
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<lyric number=\"1\"><syllabic>single</syllabic><text>This</text>"
            "<elision>‿</elision><syllabic>single</syllabic><text>is</text></lyric></note>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, lyric_08)
    {
        //@08. lyric. placement

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n c4 q (lyric 2 \"This\" below))"
            ")))" );
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
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<lyric number=\"2\" placement=\"below\"><syllabic>single</syllabic>"
            "<text>This</text></lyric></note>";
        CHECK( check_result(source, expected) );
    }

    //@ measure -------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, measure_01)
    {
        //@01. measure. StaffObjs for start and end of measure correctly identified

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/017-clef-change-at-start-of-measure.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pMD);

        string expected ="</note></measure>";
        CHECK( check_result_contains(source, expected, "</note>", "<measure") );

        expected = "<measure number=\"2\">"
            "<attributes><clef><sign>F</sign><line>4</line></clef></attributes>"
            "<note><pitch><step>D</step><octave>3</octave>";
        CHECK( check_result_contains(source, expected, "<measure number=\"2\">", "</pitch>") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, measure_02)
    {
        //@02. measure. as test 01 but with barline type=double

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/018-clef-change-barline-double.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);

        string source = exporter.get_source(pMD);
        string expected = "<octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "<barline><bar-style>light-light</bar-style></barline>"
            "</measure>";
        CHECK( check_result_contains(source, expected, "<octave>4", "<measure") );

        expected = "<measure number=\"2\">"
            "<attributes><clef><sign>F</sign><line>4</line></clef></attributes>"
            "<note><pitch><step>D</step><octave>3</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type></note>"
            "</measure>";
        CHECK( check_result_contains(source, expected, "<measure number=\"2\">", "</part") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, measure_03)
    {
        //@03. measure. identify start/end of measure StaffObjs. Several clef changes

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/019-several-clef-changes.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);

        string source = exporter.get_source(pMD);
        string expected = "<measure number=\"1\">"
            "<attributes><divisions>480</divisions>"
            "<clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>1920</duration><voice>1</voice><type>whole</type></note>"
            "</measure>";
        CHECK( check_result_contains(source, expected, " number=\"1\">", "<measure") );

        expected = "<measure number=\"2\">"
            "<attributes><clef><sign>C</sign><line>3</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>1920</duration><voice>1</voice><type>whole</type></note>"
            "</measure>";
        CHECK( check_result_contains(source, expected, "<measure number=\"2\">", "<measure") );

        expected = "<measure number=\"3\">"
            "<attributes><clef><sign>C</sign><line>4</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>1920</duration><voice>1</voice><type>whole</type></note>"
            "</measure>";
        CHECK( check_result_contains(source, expected, "<measure number=\"3\">", "</part") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, measure_04)
    {
        //@04. measure. print and measure-numbering exported

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/022-measure-numbering.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part id=\"P1\"><measure number=\"1\">"
            "<print><measure-numbering>system</measure-numbering></print>";
        CHECK( check_result_contains(source, expected, "<part id=\"P1\">", "<attributes") );

        expected = "<part id=\"P2\"><measure number=\"1\">";
        CHECK( check_result_contains(source, expected, "<part id=\"P2\">", "<attributes") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, measure_05)
    {
        //@05. measure. 'implicit' and 'number' attributes

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure implicit=\"yes\" number=\"X1\">"
                "<attributes>"
                    "<divisions>4</divisions>"
                    "<clef><sign>G</sign><line>2</line></clef>"
                "</attributes>"
                "<note><rest/><duration>16</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pScore);
        string expected = "<measure implicit=\"yes\" number=\"X1\">";
        CHECK( check_result_contains(source, expected, "<measure", "<attributes>") );
    }

    //@ notations -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, notations_01)
    {
        //@01. notations. fermata

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData (clef G)"
            "(n e4 q (fermata above))"
            "(r q (fermata very-short below))"
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
            "<notations><fermata type=\"upright\"/></notations></note>";
        CHECK( check_result(source, expected) );

        ++it;   //rest
        source = exporter.get_source(*it);
        expected = "<note><rest/>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><fermata type=\"inverted\">double-angled</fermata></notations></note>";
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

    TEST_FIXTURE(MxlExporterTestFixture, notations_03)
    {
        //@03. notations. arpeggio

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/016-arpeggio.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_remove_separator_lines(true);

        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(pMD);
        string expected = "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><arpeggiate direction=\"up\"/></notations>"
            "</note>";
        CHECK( check_result_contains(source, expected, "<note><pitch><step>C", "</note>") );

        expected = "<note><chord/><pitch><step>E</step><octave>5</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><arpeggiate/></notations>"
            "</note>";
        CHECK( check_result_contains(source, expected, "<note><chord/><pitch><step>E", "</note>") );

        expected = "<note><chord/><pitch><step>G</step><octave>5</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type>"
            "<notations><arpeggiate/></notations>"
            "</note>";
        CHECK( check_result_contains(source, expected, "<note><chord/><pitch><step>G", "</note>") );
    }

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

    TEST_FIXTURE(MxlExporterTestFixture, note_04)
    {
        //@04. grace note. steal-time-previous

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/210-grace-note.xml", Document::k_format_mxl);
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
            "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><grace steal-time-previous=\"20\"/>"
                "<pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type><stem>up</stem></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch><duration>960</duration>"
                "<voice>1</voice><type>half</type><stem>down</stem></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_05)
    {
        //@05. grace note. steal-time-previous

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/211-graces-chord.xml", Document::k_format_mxl);
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
            "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><grace steal-time-following=\"20\"/>"
                "<pitch><step>D</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type><stem>up</stem></note>"
            "<note><grace/><chord/>"
                "<pitch><step>F</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type><stem>up</stem></note>"
            "<note><pitch><step>C</step><octave>5</octave></pitch><duration>480</duration>"
                "<voice>1</voice><type>quarter</type><stem>down</stem></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_06)
    {
        //@06. grace note. slash

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/grace-notes/228-grace-slash.xml", Document::k_format_mxl);
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
            "<clef><sign>G</sign><line>2</line></clef>"
            "</attributes>"
            "<note><grace steal-time-previous=\"20\" slash=\"yes\"/>"
                "<pitch><step>E</step><octave>5</octave></pitch>"
                "<voice>1</voice><type>eighth</type><stem>up</stem></note>"
            "<note><pitch><step>E</step><octave>5</octave></pitch><duration>960</duration>"
                "<voice>1</voice><type>half</type><stem>up</stem></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_07)
    {
        //@07. cue note

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/004-cue-note.xml", Document::k_format_mxl);
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
            "<clef><sign>percussion</sign></clef>"
            "</attributes>"
            "<note><cue/><unpitched><display-step>E</display-step>"
                "<display-octave>5</display-octave></unpitched><duration>1440</duration>"
                "<voice>1</voice><type>half</type><dot/><stem>down</stem></note>"
            "</measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, note_08)
    {
        //@08. note. attributes

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
                "(musicData (clef G)(n d4 q (visible no)) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoNote* pImo = static_cast<ImoNote*>(
                                        pMD->get_child_of_type(k_imo_note_regular) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<note print-object=\"no\"><pitch><step>D</step><octave>4</octave></pitch>"
            "<duration>480</duration><voice>1</voice><type>quarter</type></note>";
        CHECK( check_result(source, expected) );
    }

    //@ ornaments -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, ornaments_01)
    {
        //@01. ornaments. turn

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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

    TEST_FIXTURE(MxlExporterTestFixture, ornaments_02)
    {
        //@02. ornaments. tremolo

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<ornaments><tremolo type=\"single\">3</tremolo></ornaments>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
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
                "<notations><ornaments><tremolo type=\"single\">3</tremolo></ornaments>"
                "</notations></note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

//    TEST_FIXTURE(MxlExporterTestFixture, ornaments_xx)
//    {
//        //@xx. ornaments. wavy-line
//
//    }

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

    TEST_FIXTURE(MxlExporterTestFixture, part_list_03)
    {
        //@03. part-list. score-instrument

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id=\"P1\">"
                "<part-name>Voice</part-name>"
                "<score-instrument id=\"P1-I1\">"
                    "<instrument-name>Acoustic Guitar (steel)</instrument-name>"
                    "<instrument-sound>pluck.guitar</instrument-sound>"
                "</score-instrument>"
            "</score-part></part-list>"
            "<part id=\"P1\"><measure number='1'>"
            "</measure></part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part-list><score-part id=\"P1\">"
            "<part-name>Voice</part-name><score-instrument id=\"P1-I1\">"
            "<instrument-name>Acoustic Guitar (steel)</instrument-name>"
            "<instrument-sound>pluck.guitar</instrument-sound><solo/>"
            "</score-instrument></score-part></part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, part_list_04)
    {
        //@04. part-list. midi-instrument

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id=\"P2\">"
               "<part-name>Horn in F</part-name>"
               "<part-abbreviation>Hn.</part-abbreviation>"
               "<score-instrument id=\"P2-I2\">"
                  "<instrument-name>Horn</instrument-name>"
                  "<instrument-abbreviation>Hn</instrument-abbreviation>"
                  "<instrument-sound>brass.french-horn</instrument-sound>"
                  "<virtual-instrument>"
                     "<virtual-library>Garritan Instruments for Finale</virtual-library>"
                     "<virtual-name>002. Brass/1. French horns/French horn Plr1</virtual-name>"
                  "</virtual-instrument>"
               "</score-instrument>"
               "<midi-device>ARIA Player</midi-device>"
               "<midi-instrument id=\"P2-I2\">"
                  "<midi-channel>2</midi-channel>"
                  "<midi-program>1</midi-program>"
                  "<volume>80</volume>"
                  "<pan>4</pan>"
               "</midi-instrument>"
            "</score-part></part-list>"
            "<part id=\"P2\"><measure number='1'>"
            "</measure></part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part-list><score-part id=\"P2\">"
            "<part-name>Horn in F</part-name><part-abbreviation>Hn.</part-abbreviation>"
            "<score-instrument id=\"P2-I2\"><instrument-name>Horn</instrument-name>"
            "<instrument-abbreviation>Hn</instrument-abbreviation>"
            "<instrument-sound>brass.french-horn</instrument-sound><solo/>"
            "<virtual-instrument><virtual-library>Garritan Instruments for Finale</virtual-library>"
            "<virtual-name>002. Brass/1. French horns/French horn Plr1</virtual-name>"
            "</virtual-instrument></score-instrument><midi-device>ARIA Player</midi-device>"
            "<midi-instrument id=\"P2-I2\"><midi-channel>2</midi-channel>"
            "<midi-program>1</midi-program><volume>80</volume><pan>4</pan>"
            "</midi-instrument></score-part></part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, part_list_05)
    {
        //@05. part-list. several instruments per part

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id=\"P2\">"
             "<part-name/>"
             "<score-instrument id=\"P2-I4\">"
                "<instrument-name>Cantus 2</instrument-name>"
                "<solo/>"
             "</score-instrument>"
             "<score-instrument id=\"P2-I3\">"
                "<instrument-name>Tenor</instrument-name>"
                "<solo/>"
             "</score-instrument>"
             "<midi-instrument id=\"P2-I4\">"
                "<midi-channel>2</midi-channel>"
                "<midi-program>42</midi-program>"
                "<volume>80</volume>"
                "<pan>0</pan>"
             "</midi-instrument>"
             "<midi-instrument id=\"P2-I3\">"
                "<midi-channel>3</midi-channel>"
                "<midi-program>43</midi-program>"
                "<volume>80</volume>"
                "<pan>0</pan>"
             "</midi-instrument>"
            "</score-part></part-list>"
            "<part id=\"P2\"><measure number='1'>"
            "</measure></part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected = "<part-list><score-part id=\"P2\">"
             "<part-name/>"
             "<score-instrument id=\"P2-I4\">"
                "<instrument-name>Cantus 2</instrument-name>"
                "<solo/>"
             "</score-instrument>"
             "<score-instrument id=\"P2-I3\">"
                "<instrument-name>Tenor</instrument-name>"
                "<solo/>"
             "</score-instrument>"
             "<midi-instrument id=\"P2-I4\">"
                "<midi-channel>2</midi-channel>"
                "<midi-program>42</midi-program>"
                "<volume>80</volume>"
                "<pan>0</pan>"
             "</midi-instrument>"
             "<midi-instrument id=\"P2-I3\">"
                "<midi-channel>3</midi-channel>"
                "<midi-program>43</midi-program>"
                "<volume>80</volume>"
                "<pan>0</pan>"
             "</midi-instrument>"
            "</score-part></part-list>";
        CHECK( check_result_contains(source, expected, "part-list") );
    }

    //@ print ---------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, print_01)
    {
        //@01. print: staff-distance exported

        Document doc(m_libraryScope);
        doc.from_string(
            "<score-partwise version='3.0'>"
            "<defaults>"
                "<staff-layout>"
                  "<staff-distance>80</staff-distance>"
                "</staff-layout>"
            "</defaults>"
            "<part-list><score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<print>"
                "<staff-layout number=\"2\">"
                  "<staff-distance>93</staff-distance>"
                "</staff-layout>"
            "</print>"
            "<attributes>"
                "<staves>2</staves>"
                "<clef number=\"1\"><sign>G</sign><line>2</line></clef>"
                "<clef number=\"2\"><sign>F</sign><line>4</line></clef>"
            "</attributes>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);
        string expected =
            "<defaults>"
                "<staff-layout>"
                  "<staff-distance>80</staff-distance>"
                "</staff-layout>"
            "</defaults>";
        CHECK( check_result_contains(source, expected, "<defaults>") );

        expected =
            "<print>"
                "<staff-layout number=\"2\">"
                  "<staff-distance>93</staff-distance>"
                "</staff-layout>"
            "</print>";
        CHECK( check_result_contains(source, expected, "<print>") );
    }

    //@ rest ----------------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, rest_01)
    {
        //@01. rest. minimal test

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

    TEST_FIXTURE(MxlExporterTestFixture, rest_02)
    {
        //@02. rest. position on the staff

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text(
            "<note>"
                "<rest><display-step>E</display-step><display-octave>4</display-octave></rest>"
                "<duration>96</duration>"
                "<voice>1</voice>"
                "<type>quarter</type>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        a.set_current_divisions(96);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_rest() == true );
        ImoRest* pImo = dynamic_cast<ImoRest*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<note>"
                "<rest><display-step>E</display-step><display-octave>4</display-octave></rest>"
                "<duration>480</duration>"
                "<voice>1</voice>"
                "<type>quarter</type>"
                "</note>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
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

    TEST_FIXTURE(MxlExporterTestFixture, slur_03)
    {
        //@03. slur. placement

        Document doc(m_libraryScope);
        doc.from_string("<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name/></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes><divisions>4</divisions><clef><sign>G</sign><line>2</line></clef></attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch>"
            "<duration>4</duration><voice>1</voice><type>quarter</type>"
            "<notations><slur number=\"1\" type=\"start\" placement=\"above\"/>"
            "</notations></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch><duration>4</duration>"
            "<voice>1</voice><type>quarter</type>"
            "<notations><slur number=\"1\" type=\"stop\"/>"
            "</notations></note>"
            "</measure>"
            "</part></score-partwise>", Document::k_format_mxl );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_score(pScore);
        exporter.set_current_instrument(pInstr);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pMD);
        string expected = "<slur number=\"1\" type=\"start\" placement=\"above\"/>";
        CHECK( check_result_contains(source, expected, "slur") );
    }

    TEST_FIXTURE(MxlExporterTestFixture, slur_04)
    {
        //@04. slur. stop before start

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/028-slurs.xml", Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);

        string expected = "<slur number=\"1\" type=\"start\" placement=\"above\"/>";
        CHECK( check_result_contains(source, expected) );

        expected = "<slur number=\"2\" type=\"stop\"/>";
        CHECK( check_result_contains(source, expected) );

        expected = "<slur number=\"2\" type=\"start\" placement=\"above\"/>";
        CHECK( check_result_contains(source, expected) );

        expected = "<slur number=\"1\" type=\"stop\"/>";
        CHECK( check_result_contains(source, expected) );
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

        CHECK( check_errormsg_empty(errormsg.str()) );
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

    TEST_FIXTURE(MxlExporterTestFixture, technical_04)
    {
        //@04. technical notations. fingering: substitution, alternate

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser(errormsg);
        parser.parse_text(
            "<note>"
                "<pitch><step>A</step>"
                "<octave>3</octave></pitch>"
                "<duration>1</duration><type>quarter</type>"
                "<notations>"
                    "<technical><fingering>5</fingering>"
                        "<fingering substitution=\"yes\">3</fingering>"
                        "<fingering alternate=\"yes\">2</fingering></technical>"
                "</notations>"
            "</note>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_note() == true );
        ImoNote* pNote = dynamic_cast<ImoNote*>( pRoot );
        CHECK( pNote != nullptr );
        if (pNote)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pNote);
            string expected = "<technical><fingering>5</fingering>"
                "<fingering substitution=\"yes\">3</fingering>"
                "<fingering alternate=\"yes\">2</fingering></technical>";
            CHECK( check_result_contains(source, expected, "<technical>", "</notations>") );
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

    TEST_FIXTURE(MxlExporterTestFixture, tie_02)
    {
        //@02. tie. orientation

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/026-tied-orientation.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);

        string expected = "<tied type=\"start\" orientation=\"under\"/>";
        CHECK( check_result_contains(source, expected, "<tied type=\"start\"/", "</notations>") );

        expected = "<tied type=\"stop\"/>";
        CHECK( check_result_contains(source, expected, "<tied type=\"stop\"/", "</notations>") );
    }

    //@ time signature ------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, time_signature_01)
    {
        //@01. minimal test. beats and beat type

        Document doc(m_libraryScope);
        ImoTimeSignature* pImo =
            static_cast<ImoTimeSignature*>(ImFactory::inject(k_imo_time_signature, &doc));
        pImo->set_top_number(6);
        pImo->set_bottom_number(8);
        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        string source = exporter.get_source(pImo);
        string expected = "<attributes><time><beats>6</beats><beat-type>8</beat-type></time>";
        CHECK( check_result(source, expected) );
        delete pImo;
    }

    TEST_FIXTURE(MxlExporterTestFixture, time_signature_02)
    {
        //@02. time signature: print-style, print-object

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument (musicData"
                "(time 3 4 (visible no)(dx 52.6)) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        ImoInstrument* pInstr = pScore->get_instrument(0);
        ImoMusicData* pMD = pInstr->get_musicdata();
        ImoObj::children_iterator it = pMD->begin();    //clef G

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_current_instrument(pInstr);
        string source = exporter.get_source(*it);
        string expected = "<attributes><time relative-x=\"52.6\" print-object=\"no\">"
            "<beats>3</beats><beat-type>4</beat-type></time>";
        CHECK( check_result(source, expected) );
    }

    //@ transpose -----------------------------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, transpose_01)
    {
        //@01. transpose. minimal test

        stringstream errormsg;
        Document doc(m_libraryScope);
        XmlParser parser;
        parser.parse_text("<transpose><diatonic>-2</diatonic><chromatic>-3</chromatic>"
            "</transpose>");
        MyMxlAnalyser2 a(errormsg, m_libraryScope, &doc, &parser);
        XmlNode* tree = parser.get_tree_root();
        ImoObj* pRoot =  a.analyse_tree(tree, "string:");

        CHECK( check_errormsg_empty(errormsg.str()) );
        CHECK( pRoot && pRoot->is_transpose() == true );
        ImoTranspose* pImo = dynamic_cast<ImoTranspose*>( pRoot );
        CHECK( pImo != nullptr );
        if (pImo)
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_newlines(true);
            string source = exporter.get_source(pImo);
            string expected = "<attributes><transpose>"
                "<diatonic>-2</diatonic><chromatic>-3</chromatic></transpose>";
            CHECK( check_result(source, expected) );
        }

        a.do_not_delete_instruments_in_destructor();
        delete pRoot;
    }

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

    TEST_FIXTURE(MxlExporterTestFixture, tuplet_03)
    {
        //@03. tuplet. starts in rest

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/027-tuplet-starts-in-rest.xml",
                      Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MxlExporter exporter(m_libraryScope);
        exporter.set_remove_newlines(true);
        exporter.set_remove_separator_lines(true);
        string source = exporter.get_source(pScore);

        string expected =
            "<tuplet type=\"start\" number=\"1\" bracket=\"no\" show-number=\"actual\">"
                "<tuplet-actual><tuplet-number>6</tuplet-number></tuplet-actual>"
                "<tuplet-normal><tuplet-number>4</tuplet-number></tuplet-normal>"
            "</tuplet>";
        CHECK( check_result_contains(source, expected, "tuplet") );
    }

    //@ tests to check StaffObjs order --------------------------------------------------

    TEST_FIXTURE(MxlExporterTestFixture, staffobjs_order_01)
    {
        //@01. order. cross-staff beamed notes

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/020-single-voice-cross-staff.xml",
                      Document::k_format_mxl);
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
            "<measure number=\"1\"><attributes><divisions>480</divisions><staves>2</staves>"
            "<clef number=\"1\"><sign>G</sign><line>2</line></clef><clef number=\"2\">"
            "<sign>F</sign><line>4</line></clef></attributes><note><pitch><step>A</step>"
            "<octave>3</octave></pitch><duration>240</duration><voice>2</voice>"
            "<type>eighth</type><staff>2</staff><beam number=\"1\">begin</beam></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch><duration>240</duration>"
            "<voice>2</voice><type>eighth</type><staff>1</staff>"
            "<beam number=\"1\">continue</beam></note><note><pitch><step>A</step>"
            "<octave>3</octave></pitch><duration>240</duration><voice>2</voice>"
            "<type>eighth</type><staff>2</staff><beam number=\"1\">continue</beam>"
            "</note><note><pitch><step>E</step><octave>3</octave></pitch>"
            "<duration>240</duration><voice>2</voice><type>eighth</type><staff>1</staff>"
            "<beam number=\"1\">end</beam></note></measure>";
        CHECK( check_result(source, expected) );
    }

    TEST_FIXTURE(MxlExporterTestFixture, staffobjs_order_02)
    {
        //@02. order. cross-staff beamed notes with intermediate clef

        Document doc(m_libraryScope);
        doc.from_file(m_scores_path + "unit-tests/xml-export/021-single-voice-cross-staff-clef-change.xml",
                      Document::k_format_mxl);
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
            "<measure number=\"1\"><attributes><divisions>480</divisions><staves>2</staves>"
            "<clef number=\"1\"><sign>G</sign><line>2</line></clef><clef number=\"2\">"
            "<sign>F</sign><line>4</line></clef></attributes><note><pitch><step>A</step>"
            "<octave>3</octave></pitch><duration>240</duration><voice>2</voice>"
            "<type>eighth</type><staff>2</staff><beam number=\"1\">begin</beam></note>"
            "<note><pitch><step>E</step><octave>4</octave></pitch><duration>240</duration>"
            "<voice>2</voice><type>eighth</type><staff>1</staff>"
            "<beam number=\"1\">continue</beam></note><note><pitch><step>A</step>"
            "<octave>3</octave></pitch><duration>240</duration><voice>2</voice>"
            "<type>eighth</type><staff>2</staff><beam number=\"1\">continue</beam>"
            "</note><attributes><clef number=\"1\"><sign>F</sign><line>4</line></clef>"
            "</attributes><note><pitch><step>E</step><octave>3</octave></pitch>"
            "<duration>240</duration><voice>2</voice><type>eighth</type><staff>1</staff>"
            "<beam number=\"1\">end</beam></note></measure>";
        CHECK( check_result(source, expected) );
    }

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
