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
#include "private/lomse_document_p.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_autoclef.h"

#include <exception>
using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//Derived class to access protected members
class MyAutoClef : public AutoClef
{
protected:

public:
    MyAutoClef(ImoScore* pScore)
        : AutoClef(pScore)
    {
    }
    virtual ~MyAutoClef() {}

    //access to protected member methods
    bool my_are_there_staves_needing_clef()
    {
        find_staves_needing_clef();

        vector<bool>::iterator it;
        for (it=m_fNeedsClef.begin(); it != m_fNeedsClef.end(); ++it)
        {
            if (*it==true)
                return true;
        }
        return false;
    }
    FPitch my_max_pitch(int idx)
    {
        return m_maxPitch[idx];
    }
    FPitch my_min_pitch(int idx)
    {
        return m_minPitch[idx];
    }
    int my_num_notes(int idx)
    {
        return m_numNotes[idx];
    }

};

//=======================================================================================
// AutoClef tests
//=======================================================================================
class AutoClefTestFixture
{
public:
    LibraryScope m_libraryScope;
    string m_scores_path;

    AutoClefTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
        m_libraryScope.get_musicxml_options()->use_default_clefs(false);
    }

    ~AutoClefTestFixture()    //TearDown fixture
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

    inline void failure_header()
    {
        cout << endl << "*** Failure in " << test_name() << ":" << endl;
    }

    bool check_to_string(Document& doc, const string& expected)
    {
        if (doc.to_string() != expected)
        {
            failure_header();
            cout << "    original: " << doc.to_string() << endl;
            cout << "    expected: " << expected << endl;
            return false;
        }
        else
            return true;
    }

};

//---------------------------------------------------------------------------------------
SUITE(AutoClefTest)
{

    TEST_FIXTURE(AutoClefTestFixture, autoclef_01)
    {
        //@01. staves without notes do not require clef

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
            "(musicData (metronome q 55) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == false );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_02)
    {
        //@02. staves with notes and clef do not require clef

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
           "(instrument (staves 2) (musicData "
           "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
           "(n e4 e g+ p1 v1)(n g4 e g- v1)"
           "(n c3 e g+ p2 v2)(n e3 e g- v2)(n g3 e g+ v2)(n c4 e g- v2)"
           "(barline)"
           "(n a3 q p2 v2)(n e3 q v2) )))"
        );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == false );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_03)
    {
        //@03. staves with notes but no clef require clef

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
            "(musicData (n c4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == true );
        CHECK( ac.my_max_pitch(0) == FPitch(0, 4, 0) );     //c4
        CHECK( ac.my_min_pitch(0) == FPitch(0, 4, 0) );     //c4
        CHECK( ac.my_num_notes(0) == 1 );
//        cout << test_name() << endl;
//        cout << doc.to_string() << endl;
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_04)
    {
        //@04. one staff requires clef

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
           "(instrument (staves 2) (musicData "
           "(clef G p1)(key C)(time 2 4)"
           "(n e4 e g+ p1 v1)(n g4 e g- v1)"
           "(n c3 e g+ p2 v2)(n e3 e g- v2)(n g3 e g+ v2)(n c4 e g- v2)"
           "(barline)"
           "(n a3 q p2 v2)(n e3 q v2) )))"
        );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == true );
        CHECK( ac.my_max_pitch(1) == FPitch(0, 4, 0) );     //c4
        CHECK( ac.my_min_pitch(1) == FPitch(0, 3, 0) );     //c3
        CHECK( ac.my_num_notes(1) == 6 );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_05)
    {
        //@05. staves with unpitched notes do not require clef (LDP)

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
            "(musicData (n * q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == false );
    }


    TEST_FIXTURE(AutoClefTestFixture, autoclef_06)
    {
        //@06. staves with notes but no clef require clef (MusicXML)


        Document doc(m_libraryScope);
        doc.from_string("<?xml version='1.0' encoding='utf-8'?>"
            "<!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 3.0 Partwise//EN' "
                "'http://www.musicxml.org/dtds/partwise.dtd'>"
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == true );
        CHECK( ac.my_max_pitch(0) == FPitch(0, 4, 0) );     //c4
        CHECK( ac.my_min_pitch(0) == FPitch(0, 4, 0) );     //c4
        CHECK( ac.my_num_notes(0) == 1 );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_07)
    {
        //@07. staves with unpitched notes do not require clef (MusicXML)

        Document doc(m_libraryScope);
        doc.from_string("<?xml version='1.0' encoding='utf-8'?>"
            "<!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 3.0 Partwise//EN' "
                "'http://www.musicxml.org/dtds/partwise.dtd'>"
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
            "</attributes>"
            "<note><unpitched/><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        CHECK( ac.my_are_there_staves_needing_clef() == false );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_20)
    {
        //@20. fix LDP score. Missing clef in first staff

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) (instrument"
            "(musicData (n c4 q) )))");
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        ac.do_autoclef();

        string expected("(lenmusdoc (vers 0.0)(content (score "
              "(vers 2.0)(instrument P1 (staves 1)(musicData (clef G p1)(n c4 q v1 p1))))))" );
        CHECK( check_to_string(doc, expected) );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_21)
    {
        //@21. fix LDP score. Missing clef in second staff

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)"
           "(instrument#100 (staves 2) (musicData "
           "(clef G p1)(key C)(time 2 4)"
           "(n e4 e g+ p1 v1)(n g4 e g- v1)"
           "(n c3 e g+ p2 v2)(n e3 e g- v2)(n g3 e g+ v2)(n c4 e g- v2)"
           "(barline)"
           "(n a3 q p2 v2)(n e3 q v2) )))"
        );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        ac.do_autoclef();

        string expected("(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
              "(instrument P1 (staves 2)(musicData (clef G p1)(clef F4 p2)"
              "(key C)(time 2 4)(n e4 e v1 p1 (beam 109 +))(n c3 e v2 p2 (beam 116 +))"
              "(n g4 e v1 p1 (beam 109 -))(n e3 e v2 p2 (beam 116 -))(n g3 e v2 p2 "
              "(beam 123 +))(n c4 e v2 p2 (beam 123 -))(barline simple)(n a3 q v2 p2)"
              "(n e3 q v2 p2))))))" );
        CHECK( check_to_string(doc, expected) );
    }

    TEST_FIXTURE(AutoClefTestFixture, autoclef_22)
    {
        //@22. fix MusicXML score. Missing clef in first staff

        Document doc(m_libraryScope);
        doc.from_string("<?xml version='1.0' encoding='utf-8'?>"
            "<!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 3.0 Partwise//EN' "
                "'http://www.musicxml.org/dtds/partwise.dtd'>"
            "<score-partwise version='3.0'><part-list>"
            "<score-part id='P1'><part-name>Music</part-name></score-part>"
            "</part-list><part id='P1'>"
            "<measure number='1'>"
            "<attributes>"
                "<divisions>1</divisions><key><fifths>0</fifths></key>"
                "<time><beats>4</beats><beat-type>4</beat-type></time>"
            "</attributes>"
            "<note><pitch><step>C</step><octave>4</octave></pitch><duration>4</duration><type>whole</type></note>"
            "</measure>"
            "</part></score-partwise>"
            , Document::k_format_mxl);
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );

        MyAutoClef ac(pScore);
        ac.do_autoclef();

        string expected("(lenmusdoc (vers 0.0)(content (score (vers 2.0)"
              "(opt Render.SpacingOptions 2)(opt StaffLines.Truncate 3)"
              "(instrument P1 (name \"Music\")"
              "(staves 1)(musicData (clef G p1)(key C)(time 4 4)(n c4 w v1 p1)"
              "(barline simple))))))" );
        CHECK( check_to_string(doc, expected) );
    }

};

