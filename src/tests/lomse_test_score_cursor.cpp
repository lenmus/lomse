//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_compiler.h"
#include "lomse_time.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//derived class to have access to some protected methods
class MyScoreCursor : public ScoreCursor
{
public:
    MyScoreCursor(Document* pDoc, ImoScore* pScore) : ScoreCursor(pDoc, pScore) {}

    //access to some protected methods
    inline bool my_is_pointing_barline() { return is_pointing_barline(); }
    inline int my_prev_instr() { return m_prevState.instrument(); }
    inline int my_prev_staff() { return m_prevState.staff(); }
    inline int my_prev_measure() { return m_prevState.measure(); }
    inline float my_prev_time() { return m_prevState.time(); }
    inline long my_prev_id() { return m_prevState.id(); }

    //checks
    inline bool my_is_prev_before_start_of_score() {
         return m_prevState.is_before_start_of_score();
    }

};


//---------------------------------------------------------------------------------------
class ScoreCursorTestFixture
{
public:

    ScoreCursorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
    }

    ~ScoreCursorTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
    }

    void dump_cursor(ScoreCursor& cursor)
    {
        cout << endl;
        if (*cursor == NULL)
            cout << "At end";
        else
            cout << "Pointing to obj Id=" << (*cursor)->get_id();

        cout << ", time=" << cursor.time() << ", instr=" << cursor.instrument()
        << ", staff=" << cursor.staff() << ", measure=" << cursor.measure() << endl;
    }

    void create_document_1()
    {
        //clef 21L
        //key 22L
        //time 23L
        //note at 0 24L
        //rest at 64 25L
        //barline at 128 26L
        //note at 128 27L
        //end-of-score
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)"
                "(r q)(barline)(n d4 q)"
                ")))"
            "))" );
        DocCursor cursor(m_pDoc);
        m_pScore = static_cast<ImoScore*>(*cursor);
    }

    void create_document_empty()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData "
                ")))"
            "))" );
        DocCursor cursor(m_pDoc);
        m_pScore = static_cast<ImoScore*>(*cursor);
    }

    void dump_score()
    {
        ColStaffObjs* pCol = m_pScore->get_staffobjs_table();
        cout << pCol->dump();
    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
    ImoScore* m_pScore;
};

//---------------------------------------------------------------------------------------
// helper macros
#define CHECK_PREVIOUS_STATE(_cursor, _instr, _staff, _measure, _time, _id) \
                CHECK( _cursor.my_prev_instr() == _instr );                 \
                CHECK( _cursor.my_prev_staff() == _staff );                 \
                CHECK( _cursor.my_prev_measure() == _measure );             \
                CHECK( _cursor.my_prev_time() == _time );                   \
                CHECK( _cursor.my_prev_id() == _id );

#define CHECK_CURRENT_STATE(_cursor, _instr, _staff, _measure, _time, _id)  \
                CHECK( _cursor.instrument() == _instr );                    \
                CHECK( _cursor.staff() == _staff );                         \
                CHECK( _cursor.measure() == _measure );                     \
                CHECK( _cursor.time() == _time );                           \
                CHECK( _cursor.id() == _id );


//---------------------------------------------------------------------------------------
SUITE(ScoreCursorTest)
{

    //----------------------------------------------------------------------------
    // creation
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, start)
    {
        //@ initially in first object
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, access_to_state)
    {
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, score_empty_points_to_end)
    {
        //@ initially, if score empty points to end of score
        create_document_empty();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( *cursor == NULL );
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
    }

    //----------------------------------------------------------------------------
    // move_next
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_1_in_sequence)
    {
        //@ move_next, only one instr & one staff: move in sequence
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L);
        CHECK( cursor.my_is_prev_before_start_of_score() == true );

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 22L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.my_is_pointing_barline() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0f, 27L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 128.0f, 26L);

        cursor.move_next();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == false );
        CHECK( *cursor == NULL );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_1_remains_at_end)
    {
        //@ move_next, only one instr & one staff: if at end, remains at end
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        //dump_score();
        cursor.point_to(27L);       //move to last note

        cursor.move_next();
        CHECK( cursor.is_pointing_object() == false );
        CHECK( *cursor == NULL );
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L);

        cursor.move_next();
        CHECK( cursor.is_pointing_object() == false );
        CHECK( *cursor == NULL );
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L);
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_1)
//    {
//        //1. to next object. must skip objects in other instruments
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(27L);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 14L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 96.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_2)
//    {
//        //2. to next time. No object
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(33L);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.id() == 39L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_3)
//    {
//        //3. to next object. Must find right staff
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(17L);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 18L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_4)
//    {
//        //4. to next empty time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(33L);
//        cursor.move_next();
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.id() == 40L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 96.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_5)
//    {
//        //5. traversing a sequence of notes
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(35L);
//
//        cursor.move_next();
//        CHECK( cursor.id() == 36L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 16.0f) );
//
//        cursor.move_next();
//        CHECK( cursor.id() == 37L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 32.0f) );
//
//        cursor.move_next();
//        CHECK( cursor.id() == 38L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 48.0f) );
//
//        cursor.move_next();
//        CHECK( cursor.id() == 39L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_6)
//    {
//        //6. cross barline
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(41L, 1);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 46L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_7)
//    {
//        //7. cross last barline (move to end of staff)
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(25L, 1);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 2 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_8)
//    {
//        //8. from end of staff to next staff in same instrument
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(25L, 0);
//        cursor.move_next();     //move to end of staff
//        cursor.move_next();     //now at start of next staff
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 23L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_9)
//    {
//        //9. from end of staff to next instrument
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(25L, 1);
//        cursor.move_next();     //move to end of staff
//        cursor.move_next();     //now at start of next instrument
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 29L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_10)
//    {
//        //10. at end of score: remains there
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(48L, 1);
//        cursor.move_next();     //move to end of score
//        cursor.move_next();     //try to move after end of score
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 2 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_11)
//    {
//        //11. last note. move forward to end of score
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_next();     //move to end of score
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_12)
//    {
//        //12. at end of score: remains there
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_next();     //move to end of score
//        cursor.move_next();     //try to move after end of score
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_13)
//    {
//        //13. advance to empty time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(42L);
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 48.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_14)
//    {
//        //14. from empty time to existing time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(42L);
//        cursor.move_next();
//        cursor.move_next();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMoveNext_15)
//    {
//        //15. to next object at same time: c29 -> k31 -> t32 -> n33
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(29L);
//
//        cursor.move_next();
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 31L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        cursor.move_next();
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 32L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        cursor.move_next();
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 33L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }

    //----------------------------------------------------------------------------
    // ScoreCursor::move_prev() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_in_sequence)
    {
        //@ only one instr & one staff: move in sequence

        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();     //at last element
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0f, 27L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 128.0f, 26L);

        //start of tests

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( cursor.my_is_pointing_barline() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L);

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L);

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L);

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 22L);

        cursor.move_prev();
        //dump_cursor(cursor);
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_to_start)
    {
        //@ only one instr & one staff: move back to first sets previous state

        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.move_next();
        cursor.move_next();

        cursor.move_prev();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L);

        cursor.move_prev();     //at start

        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L);
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_1)
//    {
//        //1. to prev object. must skip objects in other instruments
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(14L);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 27L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_2)
//    {
//        //2. to prev time. No object
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(41L, 0);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.id() == 40L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 96.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_3)
//    {
//        //3. to prev object. Must find right staff
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(18L);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 17L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 32.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_4)
//    {
//        //4. to prev empty time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(41L, 0);
//        cursor.move_prev();     //move to 40L
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.id() == 39L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_5)
//    {
//        //5. traversing a sequence of notes
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(39L);
//
//        cursor.move_prev();
//        CHECK( cursor.id() == 38L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 48.0f) );
//
//        cursor.move_prev();
//        CHECK( cursor.id() == 37L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 32.0f) );
//
//        cursor.move_prev();
//        CHECK( cursor.id() == 36L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 16.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_6)
//    {
//        //6. to last object at same time (must skip clef)
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(36L);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.id() == 35L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_7)
//    {
//        //7. to prev object at same time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(35L);
//        cursor.move_prev();
//        CHECK( cursor.id() == 31L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.id() == 30L );
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_8)
//    {
//        //8. cross barline
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(46L);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 41L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 128.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_9)
//    {
//        //9. cross last barline (move from end of staff)
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(25L, 0);
//        cursor.move_next();     //move to end of staff
//        cursor.move_prev();     //test: move back
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 25L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 128.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_10)
//    {
//        //10. from start of staff to prev staff in same instrument
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(23L);
//
//        cursor.move_prev();
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 2 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        cursor.move_prev();
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 25L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 128.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_11)
//    {
//        //11. from start of staff to prev instrument:
//        //    c29 (instr 1, staff 0) -> eos (instr 0, staff 1) -> b25 (staff 1)
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(29L);
//
//        cursor.move_prev();
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 2 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        cursor.move_prev();
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 25L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 1 );
//        CHECK( is_equal_time(cursor.time(), 128.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_12)
//    {
//        //12. at start of score: remains there
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_13)
//    {
//        //13. from end of score to last note
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_next();     //move to end of score
//        cursor.move_prev();     //test: move back
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 22L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_14)
//    {
//        //14. back from empty time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(42L);
//        cursor.move_next();     //to empty time
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 42L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorMovePrev_15)
//    {
//        //15. from object to empty time
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to_barline(48L, 0);
//
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == false );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//
//        cursor.move_prev();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 42L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }

    //----------------------------------------------------------------------------
    // ScoreCursor::point_to() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorPointTo)
    {
        //@ point to an object, by object ID, does it ok
        create_document_1();
//        dump_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(25L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L);
//        dump_cursor(cursor);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, PointTo_NotFound)
    {
        //@ point to an non-existing object, by object ID, moves to end of score
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(55L);
        CHECK( *cursor == NULL );
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, at_end_PointTo)
    {
        //@ when at end, point to an object by object ID does it ok
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(55L);   //move to end (invalid object)
        cursor.point_to(25L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, is_pointing_barline_method)
    {
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);       //move to barline
        CHECK( cursor.is_pointing_object() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L);
        CHECK( cursor.my_is_pointing_barline() == true );
    }

//
//    //----------------------------------------------------------------------------
//    // ScoreCursor::skip_clef_key_time() -----------------------------------------
//    //----------------------------------------------------------------------------
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_1)
//    {
//        //1. skip objects
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.skip_clef_key_time();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 26L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_2)
//    {
//        //2. nothing to skip: remains at n7
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(7L);
//        cursor.skip_clef_key_time();
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 7L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }
//
//    //----------------------------------------------------------------------------
//    // ScoreCursor: save / restore state -----------------------------------------
//    //----------------------------------------------------------------------------
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_GetState)
//    {
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(42L);
//        //dump_cursor(cursor);
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == 42L );
//        CHECK( pState->instrument() == 1 );
//        CHECK( pState->measure() == 1 );
//        CHECK( pState->staff() == 0 );
//        CHECK( is_equal_time(pState->time(), 0.0f) );
//        delete pState;
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_GetStateAtEndOfScore)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_next();     //move to end of score
//        //dump_cursor(cursor);
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == -1L );
//        CHECK( pState->instrument() == 0 );
//        CHECK( pState->measure() == 0 );
//        CHECK( pState->staff() == 0 );
//        CHECK( is_equal_time(pState->time(), 64.0f) );
//        delete pState;
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_RestoreState)
//    {
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(42L);
//        DocCursorState* pState = cursor.get_state();
//        cursor.start_of_content();      //move to antoher place
//        cursor.restore(pState);
//        //dump_cursor(cursor);
//        CHECK( cursor.is_pointing_object() == true );
//        CHECK( cursor.id() == 42L );
//        CHECK( cursor.instrument() == 1 );
//        CHECK( cursor.measure() == 1 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//        delete pState;
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_RestoreStateAtEndOfScore)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (clef#7 G) (n#8 c4 q))))))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.move_next();     //move to end of score
//        DocCursorState* pState = cursor.get_state();
//        cursor.start_of_content();      //move to antoher place
//        cursor.restore(pState);
//        //dump_cursor(cursor);
//        CHECK( pState->get_id() == -1L );
//        CHECK( pState->instrument() == 0 );
//        CHECK( pState->measure() == 0 );
//        CHECK( pState->staff() == 0 );
//        CHECK( is_equal_time(pState->time(), 64.0f) );
//        delete pState;
//    }
//
//    //----------------------------------------------------------------------------
//    // ScoreCursor: reset_and_point_to -------------------------------------------
//    //----------------------------------------------------------------------------
//
//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursor_ResetAndPointTo)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        cursor.reset_and_point_to(7L);
//        //cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(n c4 q)" );
//    }

    //----------------------------------------------------------------------------
    // edition related operations
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, reposition_after_delete)
    {
        //@ when pointed staffobj is deleted, go to next sttaffobj
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        //dump_score();
        cursor.point_to(24L);       //move to first note
        ImoNote* pImo = static_cast<ImoNote*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);
        m_pScore->close();
        //dump_score();

        cursor.update_after_deletion();

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L);
    }

}
