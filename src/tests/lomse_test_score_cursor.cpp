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
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//derived class to have access to some protected methods
class MyScoreCursor : public ScoreCursor
{
public:
    MyScoreCursor(Document* pDoc, ImoScore* pScore)
        : ScoreCursor(pDoc, pScore)
    {
        set_auto_refresh(false);
    }

    //access to some protected methods
    inline bool my_is_pointing_barline() { return p_iter_object_is_barline(); }
    inline int my_prev_instr() { return m_prevState.instrument(); }
    inline int my_prev_staff() { return m_prevState.staff(); }
    inline int my_prev_measure() { return m_prevState.measure(); }
    inline float my_prev_time() { return m_prevState.time(); }
    inline long my_prev_id() { return m_prevState.id(); }
    inline long my_prev_ref_id() { return m_prevState.ref_obj_id(); }

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

    void dump_cursor_state(ScoreCursorState* pState)
    {
         cout << "Score state: instr=" << pState->instrument()
             << ", staff=" << pState->staff()
             << ", meas=" << pState->measure()
             << ", time=" << pState->time()
             << ", id=" << pState->id()
             << ", ref_id=" << pState->ref_obj_id()
             << endl;
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
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_2()
    {
        //An score with empty places in first staff (after 26L, an in second measure)
//       (score#15 (vers 1.6)
//       (instrument#19 (staves 2) (musicData#20
//       (clef#21 G p1)(clef#22 F4 p2)(key#23 C)(time#24 2 4)
//       (n#25 e4 e g+ p1)(n#26 g4 e g-)(goBack#32 start)
//       (n#33 c3 e g+ p2)(n#34 e3 e g-)(n#40 g3 e g+)(n#41 c4 e g-)
//       (barline#47)
//       (n#48 a3 q p2)(n#49 e3 q)
//       (barline#50) )))

        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
               "(n c3 e g+ p2)(n e3 e g-)(n g3 e g+)(n c4 e g-)"
               "(barline)"
               "(n a3 q p2)(n e3 q)"
               "(barline) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_3()
    {
        //As score 2 but without last barline
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
               "(n c3 e g+ p2)(n e3 e g-)(n g3 e g+)(n c4 e g-)"
               "(barline)"
               "(n a3 q p2)(n e3 q) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_4()
    {
        //As score with emptypositions defined by time grid
        // (score#15 (vers 1.6)
        //    (instrument#19 (staves 2) (musicData#20
        //      (clef#21 G p1)(clef#22 F4 p2)(key#23 C)(time#24 2 4)
        //      (n#25 e4 e g+ p1)(n#26 g4 e g-)(goBack start)
        //      (n#33 c3 e g+ p2)(n#34 e3 e g-)(n#40 g3 q)
        //      (barline#41) )))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
               "(n c3 e g+ p2)(n e3 e g-)(n g3 q)"
               "(barline) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//        cout << m_pDoc->to_string(k_save_ids) << endl;
    }

    void create_document_empty_score()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData "
                ")))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
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
#define CHECK_PREVIOUS_STATE(_cursor, _instr, _staff, _measure, _time, _id, _refId) \
                CHECK( _cursor.my_prev_instr() == _instr );                 \
                CHECK( _cursor.my_prev_staff() == _staff );                 \
                CHECK( _cursor.my_prev_measure() == _measure );             \
                CHECK( _cursor.my_prev_time() == _time );                   \
                CHECK( _cursor.my_prev_id() == _id );                       \
                CHECK( _cursor.my_prev_ref_id() == _refId );

#define CHECK_CURRENT_STATE(_cursor, _instr, _staff, _measure, _time, _id, _refId) \
                CHECK( _cursor.instrument() == _instr );                    \
                CHECK( _cursor.staff() == _staff );                         \
                CHECK( _cursor.measure() == _measure );                     \
                CHECK( _cursor.time() == _time );                           \
                CHECK( _cursor.id() == _id );                               \
                CHECK( _cursor.staffobj_id_internal() == _refId );          \
                CHECK( _cursor.is_at_end_of_score() == false );

#define CHECK_CURRENT_STATE_AT_END_OF_STAFF(_cursor, _instr, _staff, _measure, _time)  \
                CHECK( _cursor.instrument() == _instr );                    \
                CHECK( _cursor.staff() == _staff );                         \
                CHECK( _cursor.measure() == _measure );                     \
                CHECK( _cursor.time() == _time );                           \
                CHECK( _cursor.staffobj_id_internal() == -1L );             \
                CHECK( _cursor.id() == k_cursor_at_end_of_staff );

#define CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(_cursor, _instr, _staff, _measure, _time)  \
                CHECK( _cursor.my_prev_instr() == _instr );                 \
                CHECK( _cursor.my_prev_staff() == _staff );                 \
                CHECK( _cursor.my_prev_measure() == _measure );             \
                CHECK( _cursor.my_prev_time() == _time );                   \
                CHECK( _cursor.my_prev_id() == k_cursor_at_end_of_staff );  \
                CHECK( _cursor.my_prev_ref_id() == -1L );

#define CHECK_STATE(_state, _instr, _staff, _measure, _time, _id, _refId)   \
                CHECK( _state->instrument() == _instr );                    \
                CHECK( _state->staff() == _staff );                         \
                CHECK( _state->measure() == _measure );                     \
                CHECK( is_equal_time(_state->time(), _time) );              \
                CHECK( _state->ref_obj_id() == _refId );                    \
                CHECK( _state->id() == _id );

//---------------------------------------------------------------------------------------
SUITE(ScoreCursorTest)
{

    //----------------------------------------------------------------------------
    // creation
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, start)
    {
        //1.a initially in first object
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, access_to_state)
    {
        //1.b initially in first instrument, first staff, timepos 0, measure 0
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, score_empty_points_to_end)
    {
        //2. initially, if score empty points to end of score
        create_document_empty_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::move_prev() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_in_sequence)
    {
        //1. only one instr & one staff: move in sequence

        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();
        cursor.move_next();     //at last element
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 128.0f, 26L, 26L);

        //start of tests

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK( cursor.my_is_pointing_barline() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 22L, 22L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_to_start)
    {
        //2. only one instr & one staff: move back to first sets previous state

        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.move_next();
        cursor.move_next();

        cursor.move_prev();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);

        cursor.move_prev();     //at start

        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
        CHECK( cursor.my_is_prev_before_start_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_3)
    {
        //3. to prev object. must skip objects in other instruments
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();
        cursor.point_to(41L);   //last note staff 2, measure 1

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 64.0f, 40L, 40L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 32.0f, 34L, 34L);

        cursor.move_prev();     //must skip note in staff 1
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 32.0f, 34L, 34L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 33L, 33L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_4)
    {
        //4. back from object to empty place
        //5. back from empty place to prev empty place
        //6. back from empty place to existing time
        //7. back from object to existing time. Skips objects in other staff
        create_document_4();
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to_barline(41L, 0);   //4. prev.state: from object to empty place

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 41L, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 96.0f, k_cursor_at_empty_place, 41L);

        cursor.move_prev();     //5. prev.state: from empty place to prev empty place
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0f, k_cursor_at_empty_place, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 41L);

        cursor.move_prev();     //6. prev.state: from empty place to existing time
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);

        cursor.move_prev();     //7. prev.state: skip objects in other staff
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 25L, 25L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_8)
    {
        //8. back from object in second staff to barline. Continues in second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(48L);   //note after barline, staff 2

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0f, 48L, 48L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 128.0f, 47L, 47L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_9)
    {
        //9. from start of staff to end of previous staff in same instrument (barline)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);   //clef in instrument 1, staff 2

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_10)
    {
        //10. from start of staff to end of previous staff in same instrument (note)
        create_document_3();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);   //clef in instrument 1, staff 2

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 256.0f);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_11)
    {
        //11. from end of staff to empty place
        create_document_3();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);   //clef in instrument 1, staff 2

        cursor.move_prev();     //move to end of staff. prev state is empty pos

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 192.0f, k_cursor_at_empty_place, 49L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_12)
    {
        //12. from end of staff to object
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);   //clef in instrument 1, staff 2

        cursor.move_prev();     //move to end of staff. prev state is barline

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 256.0f, 50L, 50L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_13)
    {
        //13. to prev object at same time: note -> time --> key -> clef
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(33L);   //first note in instrument 1, staff 2

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 33L, 33L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 24L, 24L);

        cursor.move_prev();     //to time. prev state is key signature

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 24L, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);

        cursor.move_prev();     //to key. prev state is clef

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 22L, 22L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_14)
    {
        //14. at start of score: remains there
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(25L);   //first note in instrument 1, staff 1

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);

        cursor.move_prev();     //to time. prev state is key signature

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);

        cursor.move_prev();     //to key. prev state is clef (start of score)

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);

        cursor.move_prev();     //to clef (start of score). prev state is before start of score

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
        CHECK_PREVIOUS_STATE(cursor, -1, -1, -1, -1.0f, k_cursor_before_start_of_child, -1L);

        cursor.move_prev();     // remains at start of score

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
        CHECK_PREVIOUS_STATE(cursor, -1, -1, -1, -1.0f, k_cursor_before_start_of_child, -1L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_15)
    {
        //15. from start of staff to end of previous staff in prev instrument
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();

        cursor.point_to(56L);   //clef instr 2, staff 1
                                //prev state is end of staff 2, instr 1

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 1, 0, 0, 0.0f, 56L, 56L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);

        cursor.move_prev();     //to end of prev staff, prev state is barline 53L

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 1, 256.0f, 53L, 53L);
    }

    //----------------------------------------------------------------------------
    // ScoreCursor::point_to() --------------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_1)
    {
        //1. point to an object, by object ID, does it ok
        create_document_1();
//        dump_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(25L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
//        cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_2)
    {
        //2. point to an object, by object ID. Second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to(41L);

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 96.0f, 41L, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 64.0f, 40L, 40L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, PointTo_NotFound)
    {
        //3. point to an non-existing object, by object ID, moves to end of score
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(55L);
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 192.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, at_end_PointTo)
    {
        //4. when at end, point to an object by object ID does it ok
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(55L);   //move to end (invalid object)
        cursor.point_to(25L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
    }

    //----------------------------------------------------------------------------
    // move_next
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_1_in_sequence)
    {
        //1. move_next, only one instr & one staff: move in sequence
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);
        CHECK( cursor.my_is_prev_before_start_of_score() == true );

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 21L, 21L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 22L, 22L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);

        cursor.move_next();
//        cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( cursor.my_is_pointing_barline() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 128.0f, 26L, 26L);

        cursor.move_next();
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 192.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_1_remains_at_end)
    {
        //2. move_next, only one instr & one staff: if at end, remains at end
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        //dump_score();
        cursor.point_to(27L);       //move to last note

        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 192.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);

        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 192.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, 27L, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_3)
    {
        //3. to next object. must skip objects in other instruments
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();
        cursor.point_to(25L);   //first note first staff

        cursor.move_next();

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 25L, 25L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_4)
    {
        //4. to next time. No object
        create_document_4();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        dump_score();
        cursor.point_to(26L);   //second note firt staff

        cursor.move_next();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_5)
    {
        //5. from empty place to next empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);   //second note firt staff
        cursor.move_next();     //empty

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 47L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);

        cursor.move_next();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0f, k_cursor_at_empty_place, 47L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 47L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_6)
    {
        //6. from empty place to existing time
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);   //second note firt staff
        cursor.move_next();     //empty
        cursor.move_next();     //empty

        cursor.move_next();     //to barline

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 47L, 47L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 96.0f, k_cursor_at_empty_place, 47L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_7)
    {
        //7. from object in second staff to barline. Continues in second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(41L);   //last note first measure second staff

        cursor.move_next();     //to barline

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 128.0f, 47L, 47L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 96.0f, 41L, 41L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_8)
    {
        //8. cross barline
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(41L);   //last note first measure second staff
        cursor.move_next();     //to barline

        cursor.move_next();     //to first note in second measure second staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0f, 48, 48L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 128.0f, 47L, 47L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_9)
    {
        //9. cross last barline: move to end of staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(50L);   //to barline

        cursor.move_next();     //to end of staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 256.0f, 50L, 50L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_10)
    {
        //10. from end of staff to next staff in same instrument
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(50L);   //to barline
        cursor.move_next();     //to end of staff

        cursor.move_next();     //to clef F4 on second staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 22L, 22L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_11)
    {
        //11. from end of staff to next instrument
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();
        cursor.point_to(52L);   //to last note instr 1, staff 2
        cursor.move_next();     //to barline 53L
        cursor.move_next();     //to end of instr 1, staff 2
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );

        cursor.move_next();     //to clef F4 on instr 2, staff 1

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 1, 0, 0, 0.0f, 56L, 56L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_12)
    {
        //12. at end of last staff, last instr. remains there (=2)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();
        cursor.point_to(86L);   //to last note instr 2, staff 2
        cursor.move_next();     //to barline 87L

        cursor.move_next();     //to end of score

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 1, 1, 2, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 1, 256.0f, 87L, 87L);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.move_next();     //remains at end

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 1, 1, 2, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 1, 256.0f, 87L, 87L);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_13)
    {
        //13. traversing a sequence of notes to end of score (=3)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//        dump_score();

        cursor.point_to(62L);   //first note in instr 2, staff 2
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 0.0f, 62L, 62L);
        //CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 0.0f,);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 16.0f, 63L, 63L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 0.0f, 62L, 62L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 32.0f, 64L, 64L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 16.0f, 63L, 63L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 48.0f, 65L, 65L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 32.0f, 64L, 64L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 64.0f, 75L, 75L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 48.0f, 65L, 65L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 96.0f, 76L, 76L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 64.0f, 75L, 75L);

        cursor.move_next();     //to barline 82L
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 128.0f, 82L, 82L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 96.0f, 76L, 76L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 128.0f, 85L, 85L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 0, 128.0f, 82L, 82L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 192.0f, 86L, 86L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 1, 128.0f, 85L, 85L);

        cursor.move_next();     //to barline 87L
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 256.0f, 87L, 87L);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 1, 192.0f, 86L, 86L);

        cursor.move_next();     //to end of score
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 1, 1, 2, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 1, 1, 1, 256.0f, 87L, 87L);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_14)
    {
        //14. last note. move forward to end of staff
        create_document_3();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(49L);   //last note staff 2

        cursor.move_next();     //to end of staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 1, 256.0f);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 1, 192.0f, 49L, 49L);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_15)
    {
        //15. to next object at same time: clef -> key -> time -> note
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);   //clef in staff 2

        cursor.move_next();     //key
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 22L, 22L);

        cursor.move_next();     //time
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 24L, 24L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);

        cursor.move_next();     //note
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 33L, 33L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 0.0f, 24L, 24L);
    }

    //----------------------------------------------------------------------------
    // other methods
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, iter_object_is_barline_1)
    {
        //1. iter_object_is_barline is true when pointing barline
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);       //move to barline

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0f, 26L, 26L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK( (*cursor)->is_barline() == true );
        CHECK( cursor.my_is_pointing_barline() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, iter_object_is_barline_2)
    {
        //2. iter_object_is_barline is false when not pointing barline
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(25L);       //rest

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK( cursor.my_is_pointing_barline() == false );
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_1)
//    {
//        //1. skip objects
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        cursor.point_to(22L);
//        cursor.skip_clef_key_time();
//        //cout << cursor.dump_cursor();
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
//        //cout << cursor.dump_cursor();
//        CHECK( cursor.id() == 7L );
//        CHECK( cursor.instrument() == 0 );
//        CHECK( cursor.measure() == 0 );
//        CHECK( cursor.staff() == 0 );
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//    }

    //----------------------------------------------------------------------------
    // ScoreCursor: get / restore state ------------------------------------------
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, get_state)
    {
        //1. get state: pointing object
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "90013-two-instruments-four-staves.lms" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        dump_score();
        cursor.point_to(52L);   //first half note, instr 1, staff 2, measure 2

        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );

//        dump_cursor_state(pState);
        CHECK_STATE(pState, 0, 1, 1, 128.0f, 52L, 52L);
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_state_at_end_of_score)
    {
        //2. get state: pointing to end of score
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(50L);   //to barline
        cursor.move_next();     //to end of staff

        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );

//        dump_cursor_state(pState);
        CHECK_STATE(pState, 0, 0, 2, 256.0f, k_cursor_at_end_of_staff, -1L);
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_state_at_empty_place)
    {
        //3. get state: pointing to empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);   //second note firt staff
        cursor.move_next();     //empty place

        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );

//        dump_cursor_state(pState);
        CHECK_STATE(pState, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 47L);
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, restore_state)
    {
        //4. restore state: pointing object
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(41L);   //last note in staff 2, measure 1
        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );
        cursor.point_to(21L);   //clef in staff 1

        cursor.restore_state(pState);

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 96.0f, 41L, 41L);
        CHECK_PREVIOUS_STATE(cursor, 0, 1, 0, 64.0f, 40L, 40L);
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, restore_state_at_end_of_score)
    {
        //5. restore state: pointing to end of score
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(50L);   //to barline
        cursor.move_next();     //to end of staff
        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );
        cursor.point_to(21L);   //clef in staff 1

        cursor.restore_state(pState);

//        dump_cursor_state(pState);
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 256.0f, 50L, 50L);
        delete pState;
    }

    TEST_FIXTURE(ScoreCursorTestFixture, restore_state_to_empty_place)
    {
        //6. restore state: pointing to empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);   //second note firt staff
        cursor.move_next();     //to empty place
        ScoreCursorState* pState = static_cast<ScoreCursorState*>( cursor.get_state() );
        cursor.point_to(50L);   //to barline

        cursor.restore_state(pState);

//        dump_cursor_state(pState);
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0f, k_cursor_at_empty_place, 47L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 32.0f, 26L, 26L);
        delete pState;
    }

    //----------------------------------------------------------------------------
    // integrity preservation after score modifications
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_1)
    {
        //1. after deletion. previous object exists
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);       //move to first note
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->delete_staffobj(pImo);
//        m_pScore->close();
        //dump_score();

        cursor.refresh();

        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 25L, 25L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_2)
    {
        //2. after deletion. previous is before start
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(21L);       //clef in staff 1
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->delete_staffobj(pImo);
        m_pScore->close();
        //dump_score();

        cursor.refresh();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE(cursor, -1, -1, -1, -1.0f, k_cursor_before_start_of_child, -1L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_3)
    {
        //3. after deletion. previous is end of previous staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(22L);       //clef in staff 2
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->delete_staffobj(pImo);
        m_pScore->close();
        //dump_score();

        cursor.refresh();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);
        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_4)
//    {
//        //4. after deletion. previous is empty place
//        create_document_2();
//        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(22L);       //clef in staff 2
//        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
//        ImoInstrument* pInstr = m_pScore->get_instrument(0);
//        pInstr->delete_staffobj(pImo);
//        m_pScore->close();
//        //dump_score();
//
//        cursor.refresh();
//
////        cout << cursor.dump_cursor();
//        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0f, 23L, 23L);
//        CHECK_PREVIOUS_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
//    }

//    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_5)
//    {
//        //5. after deletion. next is end of score
//        create_document_3();
//        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(49L);       //clef in staff 2
//        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
//        ImoInstrument* pInstr = m_pScore->get_instrument(0);
//        pInstr->delete_staffobj(pImo);
//        m_pScore->close();
//        //dump_score();
//
//        cursor.refresh();
//
////        cout << cursor.dump_cursor();
//        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 1, 192.0f);
//        CHECK( cursor.is_at_end_of_score() == true );
//        CHECK_PREVIOUS_STATE(cursor, 0, 1, 1, 128.0f, 48L, 48L);
//    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_6)
    {
        //6. after deleting the only existing object.
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(score (vers 1.6)(instrument (musicData (clef G))))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>( *cursor );
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->delete_staffobj(pImo);
        m_pScore->close();
        //dump_score();

        cursor.refresh();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        CHECK_PREVIOUS_STATE(cursor, -1, -1, -1, -1.0f, k_cursor_before_start_of_child, -1L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_7)
    {
        //7. after insertion. current object exists
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(27L);       //first note second measure
        ImoStaffObj* pImo = static_cast<ImoStaffObj*>(
                                ImFactory::inject_note(m_pDoc,3,4,k_32th) );
        long id = pImo->get_id();
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        ImoStaffObj* pPos = static_cast<ImoStaffObj*>( cursor.staffobj_internal() );
        pInstr->insert_staffobj(pPos, pImo);
        m_pScore->close();
//        dump_score();

        cursor.refresh();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 136.0f, 27L, 27L);
        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, id, id);
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_8)
//    {
//        //8. after insertion. current is empty place
//        create_document_2();
//        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(26L);       //2nd note, staff 1, measure 1
//        cursor.move_next();
//        cout << cursor.dump_cursor();
//
//        ImoStaffObj* pImo = static_cast<ImoStaffObj*>(
//                                ImFactory::inject_note(m_pDoc,3,4,k_32th) );
//        long id = pImo->get_id();
//        ImoInstrument* pInstr = m_pScore->get_instrument(0);
//        ImoStaffObj* pPos = static_cast<ImoStaffObj*>( cursor.staffobj_internal() );
//        pInstr->insert_staffobj(pPos, pImo);
//        m_pScore->close();
//        dump_score();
//        cout << m_pDoc->to_string(k_save_ids) << endl;
//
//        cursor.refresh();
//
//        cout << cursor.dump_cursor();
//        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 136.0f, 27L, 27L);
//        CHECK_PREVIOUS_STATE(cursor, 0, 0, 1, 128.0f, id, id);
//    }

}
