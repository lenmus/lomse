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
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_compiler.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ScoreCursorTestFixture
{
public:

    ScoreCursorTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
        m_scores_path = "../../../test-scores/";        //linux CodeBlobks
        //m_scores_path = "../../../../test-scores/";        //windows MS Visual studio .NET
    }

    ~ScoreCursorTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    void dump_cursor(DocCursor& cursor)
    {
        cout << endl;
        if (cursor.is_at_end_of_child())
            cout << "At end";
        else
            cout << (*cursor)->to_string() << ". Id=" << (*cursor)->get_id();

        cout << ", time=" << cursor.time() << ", instr=" << cursor.instrument()
        << ", staff=" << cursor.staff() << ", segment=" << cursor.segment()
        << ", pointing-obj=" << (cursor.is_pointing_object() ? "yes" : "no") << endl;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(ScoreCursorTest)
{

    //----------------------------------------------------------------------------
    // ScoreCursor::start() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorStart)
    {
        //start: initially in first instrument, first staff, after prolog,
        //timepos 0, segment 0
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        CHECK( (*cursor)->to_string() == "(n c4 q)" );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::point_to() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorPointTo)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        CHECK( (*cursor)->to_string() == "(r q)" );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorPointTo_NotFound)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(2L);
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorPointToBarline)
    {
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(41L, 1);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 41L );
        CHECK( (*cursor)->to_string() == "(barline )" );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 128.0f) );
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::move_next() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_1)
    {
        //1. to next object. must skip objects in other instruments
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(13L);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 14L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 96.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_2)
    {
        //2. to next time. No object
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(33L);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( (*cursor)->get_id() == 39L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_3)
    {
        //3. to next object. Must find right staff
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(17L);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 18L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_4)
    {
        //4. to next empty time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(33L);
        cursor.move_next();
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( (*cursor)->get_id() == 40L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 96.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_5)
    {
        //5. traversing a sequence of notes
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(35L);

        cursor.move_next();
        CHECK( (*cursor)->get_id() == 36L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 16.0f) );

        cursor.move_next();
        CHECK( (*cursor)->get_id() == 37L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 32.0f) );

        cursor.move_next();
        CHECK( (*cursor)->get_id() == 38L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 48.0f) );

        cursor.move_next();
        CHECK( (*cursor)->get_id() == 39L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_6)
    {
        //6. cross barline
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(41L, 1);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 46L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_7)
    {
        //7. cross last barline (move to end of staff)
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(25L, 1);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 2 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_8)
    {
        //8. from end of staff to next staff in same instrument
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(25L, 0);
        cursor.move_next();     //move to end of staff
        cursor.move_next();     //now at start of next staff
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 9L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_9)
    {
        //9. from end of staff to next instrument
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(25L, 1);
        cursor.move_next();     //move to end of staff
        cursor.move_next();     //now at start of next instrument
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 29L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_10)
    {
        //10. at end of score: remains there
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(48L, 1);
        cursor.move_next();     //move to end of score
        cursor.move_next();     //try to move after end of score
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 2 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_11)
    {
        //11. last note. move forward to end of score
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_next();     //move to end of score
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_12)
    {
        //12. at end of score: remains there
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_next();     //move to end of score
        cursor.move_next();     //try to move after end of score
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_13)
    {
        //13. advance to empty time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(42L);
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 48.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_14)
    {
        //14. from empty time to existing time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(42L);
        cursor.move_next();
        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_15)
    {
        //15. to next object at same time: c29 -> k31 -> t32 -> n33
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(29L);

        cursor.move_next();
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 31L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );

        cursor.move_next();
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 32L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );

        cursor.move_next();
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 33L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::move_prev() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_1)
    {
        //1. to prev object. must skip objects in other instruments
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(14L);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 13L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_2)
    {
        //2. to prev time. No object
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(41L, 0);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( (*cursor)->get_id() == 40L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 96.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_3)
    {
        //3. to prev object. Must find right staff
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(18L);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 17L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 32.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_4)
    {
        //4. to prev empty time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(41L, 0);
        cursor.move_prev();     //move to 40L
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( (*cursor)->get_id() == 39L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_5)
    {
        //5. traversing a sequence of notes
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(39L);

        cursor.move_prev();
        CHECK( (*cursor)->get_id() == 38L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 48.0f) );

        cursor.move_prev();
        CHECK( (*cursor)->get_id() == 37L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 32.0f) );

        cursor.move_prev();
        CHECK( (*cursor)->get_id() == 36L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 16.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_6)
    {
        //6. to last object at same time (must skip clef)
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(36L);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( (*cursor)->get_id() == 35L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_7)
    {
        //7. to prev object at same time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(35L);
        cursor.move_prev();
        CHECK( (*cursor)->get_id() == 31L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( (*cursor)->get_id() == 30L );
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_8)
    {
        //8. cross barline
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(46L);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 41L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 128.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_9)
    {
        //9. cross last barline (move from end of staff)
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(25L, 0);
        cursor.move_next();     //move to end of staff
        cursor.move_prev();     //test: move back
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 25L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 128.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_10)
    {
        //10. from start of staff to prev staff in same instrument
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(9L);

        cursor.move_prev();
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 2 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );

        cursor.move_prev();
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 25L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 128.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_11)
    {
        //11. from start of staff to prev instrument:
        //    c29 (instr 1, staff 0) -> eos (instr 0, staff 1) -> b25 (staff 1)
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(29L);

        cursor.move_prev();
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 2 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );

        cursor.move_prev();
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 25L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 1 );
        CHECK( is_equal_time(cursor.time(), 128.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_12)
    {
        //12. at start of score: remains there
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_13)
    {
        //13. from end of score to last note
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_next();     //move to end of score
        cursor.move_prev();     //test: move back
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 8L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_14)
    {
        //14. back from empty time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(42L);
        cursor.move_next();     //to empty time
        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 42L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_15)
    {
        //15. from object to empty time
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to_barline(48L, 0);

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 64.0f) );

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 42L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::skip_clef_key_time() -----------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_1)
    {
        //1. skip objects
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.skip_clef_key_time();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 12L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_2)
    {
        //2. nothing to skip: remains at n7
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(7L);
        cursor.skip_clef_key_time();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 7L );
        CHECK( cursor.instrument() == 0 );
        CHECK( cursor.segment() == 0 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
    }

    //----------------------------------------------------------------------------
    // ScoreCursor: save / restore state -----------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_GetState)
    {
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(42L);
        //dump_cursor(cursor);
        DocCursorState* pState = cursor.get_state();
        CHECK( pState->get_id() == 42L );
        CHECK( pState->instrument() == 1 );
        CHECK( pState->segment() == 1 );
        CHECK( pState->staff() == 0 );
        CHECK( is_equal_time(pState->time(), 0.0f) );
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_GetStateAtEndOfScore)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_next();     //move to end of score
        //dump_cursor(cursor);
        DocCursorState* pState = cursor.get_state();
        CHECK( pState->get_id() == -1L );
        CHECK( pState->instrument() == 0 );
        CHECK( pState->segment() == 0 );
        CHECK( pState->staff() == 0 );
        CHECK( is_equal_time(pState->time(), 64.0f) );
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_RestoreState)
    {
        Document doc(*m_pLibraryScope);
        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(42L);
        DocCursorState* pState = cursor.get_state();
        cursor.start_of_content();      //move to antoher place
        cursor.restore(pState);
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->get_id() == 42L );
        CHECK( cursor.instrument() == 1 );
        CHECK( cursor.segment() == 1 );
        CHECK( cursor.staff() == 0 );
        CHECK( is_equal_time(cursor.time(), 0.0f) );
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_RestoreStateAtEndOfScore)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        cursor.point_to(8L);
        cursor.move_next();     //move to end of score
        DocCursorState* pState = cursor.get_state();
        cursor.start_of_content();      //move to antoher place
        cursor.restore(pState);
        //dump_cursor(cursor);
        CHECK( pState->get_id() == -1L );
        CHECK( pState->instrument() == 0 );
        CHECK( pState->segment() == 0 );
        CHECK( pState->staff() == 0 );
        CHECK( is_equal_time(pState->time(), 64.0f) );
        delete pState;
    }

    //----------------------------------------------------------------------------
    // ScoreCursor: reset_and_point_to -------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_ResetAndPointTo)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
        DocCursor cursor(&doc);
        cursor.enter_element();
        ++cursor;
        CHECK( (*cursor)->to_string() == "(r q)" );
        cursor.reset_and_point_to(7L);
        //cout << (*cursor)->to_string() << endl;
        CHECK( (*cursor)->to_string() == "(n c4 q)" );
    }

    //Minimum test cases for any ScoreCursor method
    //-------------------------------------------------
    //  at end of staff (no ref_object)
    //  pointing object
    //  pointing empty timepos (occupied in other staff)

}
