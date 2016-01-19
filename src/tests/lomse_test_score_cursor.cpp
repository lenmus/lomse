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
    }

    //access to some protected methods
    inline bool my_is_pointing_barline() { return p_iter_object_is_barline(); }

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
        m_pDoc = NULL;
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
        //very simple,one line score
        //(clef#23L G)(key#24L C)(time#25L 2 4)(n#26L c4 q)(r#27L q)(barline#28L)(n#29L d4 q)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)"
                "(r q)(barline)(n d4 q)"
                ")))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        //cout << m_pDoc->to_string(true) << endl;
    }

    void create_document_2()
    {
        //An score with empty places in first staff (after 27L, an in second measure)
        //(score#4 (vers 2.0)
        //(instrument#21L (staves 2) (musicData#21
        //(clef#23L G p1)(clef#24L F4 p2)(key#25L C)(time#26L 2 4)
        //(n#27L e4 e g+ p1 v1)(n#28L g4 e g- v1)
        //(n#34L c3 e g+ p2 v2)(n#35L e3 e g- v2)(n#41L g3 e g+ v2)(n#42L c4 e g- v2)
        //(barline#48L)
        //(n#49L a3 q p2 v2)(n#50L e3 q v2)
        //(barline#51L) )))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ p1 v1)(n g4 e g- v1)"
               "(n c3 e g+ p2 v2)(n e3 e g- v2)(n g3 e g+ v2)(n c4 e g- v2)"
               "(barline)"
               "(n a3 q p2 v2)(n e3 q v2)"
               "(barline) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_3()
    {
        //As score 2 but without last barline
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ p1 v1)(n g4 e g- v1)"
               "(n c3 e g+ p2 v2)(n e3 e g- v2)(n g3 e g+ v2)(n c4 e g- v2)"
               "(barline)"
               "(n a3 q p2 v2)(n e3 q v2) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_4()
    {
        //As score 3 with empty positions over note in second staff
        //    (instrument#21L (staves 2) (musicData#22L
        //      (clef#23L G p1)(clef#24L F4 p2)(key#25L C)(time#26L 2 4)
        //      (n#27L e4 e g+ p1)(n#28L g4 e g-)
        //      (n#34L c3 e g+ p2)(n#35L e3 e g-)(n#41L g3 q)
        //      (barline#42L) )))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)"
               "(instrument (staves 2) (musicData "
               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
               "(n e4 e g+ v1 p1)(n g4 e g- v1 p1)"
               "(n c3 e g+ v2 p2)(n e3 e g- v2 p2)(n g3 q v2 p2)"
               "(barline) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        //cout << m_pDoc->to_string(true) << endl;
    }

    void create_document_5()
    {
        //A score in 6/8 time signature
        // (score#4 (vers 2.0)
        //    (instrument#21L (musicData#22L
        //      (clef#23L G)(key#24L C)(time#25L 6 8)
        //      (n#26L e4 e g+)(n#27L g4 e)(n#28L c5 e g-)
        //      (n#36L c4 e g+)(n#37L e4 e)(n#38L g4 e g-)
        //      (barline#46L)
        //      (n#47L e4 e g+)(n#48L g4 e)(n#49L c5 e g-)
        //      (n#57L c4 e g+)(n#58L e4 e)(n#59L g4 e g-)
        //      (barline#67L) )))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)"
               "(instrument (musicData "
               "(clef G)(key C)(time 6 8)"
               "(n e4 e g+)(n g4 e)(n c5 e g-)"
               "(n c4 e g+)(n e4 e)(n g4 e g-)"
               "(barline)"
               "(n e4 e g+)(n g4 e)(n c5 e g-)"
               "(n c4 e g+)(n e4 e)(n g4 e g-)"
               "(barline) )))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        //cout << m_pDoc->to_string(true) << endl;
    }

    void create_document_6()
    {
        //score with chords
        //(clef#23L G)(key#24L C)(time#25L 2 4)"
        //"(chord (n#27L c4 q)(n#29L e4 q)(n#31L g4 q))(n#33L c5 q)"
        //"(chord (n#35L c4 q)(n#37L e4 q)(n#39L g4 q))"
        //"(chord (n#42L c4 q)(n#44L e4 q)(n#46L g4 q))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)"
                "(chord (n c4 q)(n e4 q)(n g4 q))(n c5 q)"
                "(chord (n c4 q)(n e4 q)(n g4 q))"
                "(chord (n c4 q)(n e4 q)(n g4 q))"
                ")))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_7()
    {
        //score with two instruments (90013)
        //(instrument (staves 2)(musicData
        //(clef#23L G p1)(clef#24L F4 p2)(key#25L D)(time#26L 2 4)
        //(n#27L f4 q v1 p1)(n#28L g4 e g+ v1 p1)(n#29L b4 e g- v1 p1)
        //(n#35L c3 e g+ v2 p2)(n#36L c3 e g- v2 p2)(n#42L d3 q v2 p2)
        //(barline#43L)
        //(n#44L c4 e g+ v1 p1)(n#45L c4 e g- v1 p1)(n#51L d4 q v1 p1)
        //(n#52L f3 h v2 p2)
        //(barline#53L) ))
        //
        //(instrument (staves 2)(musicData
        //(clef#56L G p1)(clef#57L F4 p2)(key#58L D)(time#59L 2 4)
        //(n#60L f4 q v1 p1)
        //(n#61L c3 s g+ v2 p2)(n#62L d3 s v2 p2)(n#63L e3 s v2 p2)(n#64L f3 s g- v2 p2)
        //(n#74L g3 e g+ v2 p2)(n#75L d3 e g- v2 p2)
        //(barline#81L)
        //(n#82L a4 e. v1 p1)
        //(n#83L c3 q v2 p2)(n#84L g3 q v2 p2)
        //(barline#85L) ))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string(
            "(score (vers 2.0)"
            "    (instrument (staves 2)(musicData"
            "        (clef G p1)(clef F4 p2)(key D)(time 2 4)"
            "            (n f4 q v1 p1)(n g4 e g+ v1 p1)(n b4 e g- v1 p1)"
            "            (n c3 e g+ v2 p2)(n c3 e g- v2 p2)(n d3 q v2 p2)"
            "            (barline)"
            "            (n c4 e g+ v1 p1)(n c4 e g- v1 p1)(n d4 q v1 p1)"
            "            (n f3 h v2 p2)"
            "            (barline)"
            "    ))"
            "    (instrument (staves 2)(musicData "
            "        (clef G p1)(clef F4 p2)(key D)(time 2 4)"
            "            (n f4 q v1 p1)"
            "            (n c3 s g+ v2 p2)(n d3 s v2 p2)(n e3 s v2 p2)(n f3 s g- v2 p2)(n g3 e g+ v2 p2)(n d3 e g- v2 p2)"
            "            (barline)"
            "            (n a4 e. v1 p1)"
            "            (n c3 q v2 p2)(n g3 q v2 p2)"
            "            (barline)"
            "    )))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_8()
    {
        //score with one staff and two voices
        //(score#4 (vers 2.0)(instrument#21L (musicData#22L
        //(clef#23L G p1 )(key#24L F)(time#25L 4 4)
        //(n#26L c5 q v1  (stem up) p1 )(n#30L g4 e v2  (stem down) p1 )
        //(n#31L a4 q v2  (stem down) p1 )(goFwd#27L q v1 p1)
        //(n#32L f4 e v2  (stem down) p1 )(n#28L d5 q v1  (stem up) p1 )
        //(n#33L e4 e v2  (stem down) p1 )(r#34L e v2 p1)
        //(goFwd#29L e v1 p1)(n#35L d4 q v2  (stem down) p1 )
        //(n#29L e5 e v1  (stem up) p1 )(barline#36L simple)
        //(n#37L c5 e v1 (stem up))
        //(n#38L g4 q v2 (stem down))(n#39L a4 e v2 (stem down))
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string(
            "(score (vers 2.0)(instrument (musicData "
            "    (clef G)(key F)(time 4 4)"
            "    (n c5 q v1 (stem up))(goFwd q v1)(n d5 q v1 (stem up))(n e5 e v1 (stem up))"
            "    (n g4 e v2 (stem down))(n a4 q v2 (stem down))(n f4 e v2 (stem down))"
            "        (n e4 e v2 (stem down))(r e v2)(n d4 q v2 (stem down))"
            "    (barline)"
            "    (n c5 e v1 (stem up))"
            "    (n g4 q v2 (stem down))(n a4 e v2 (stem down))"
            ")))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_9()
    {
        //two staves, only clef, key and time signature
        //(score#5L (vers 2.0)(instrument#21L (musicData#22L
        //(clef#23L G p1 )(clef#24L F4 p2 )(key#25L D)(time#26L 4 4)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string(
            "(score (vers 2.0)(instrument (staves 2)(musicData "
            "    (clef G p1)(clef F4 p2)(key D)(time 4 4)"
            ")))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void create_document_empty_score()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0) "
                "(instrument (musicData "
                ")))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

    void dump_col_staff_objs()
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

#define CHECK_CURRENT_STATE_AT_END_OF_SCORE(_cursor, _instr, _staff, _measure, _time)  \
                CHECK( _cursor.instrument() == _instr );                    \
                CHECK( _cursor.staff() == _staff );                         \
                CHECK( _cursor.measure() == _measure );                     \
                CHECK( _cursor.time() == _time );                           \
                CHECK( _cursor.staffobj_id_internal() == -1L );             \
                CHECK( _cursor.id() == k_cursor_at_end_of_child );

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

    TEST_FIXTURE(ScoreCursorTestFixture, creation_001)
    {
        //001. initially in first object, in first instrument, first staff,
        //     timepos 0, measure 0
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, creation_002)
    {
        //002. initially, if score empty points to end of score
        create_document_empty_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
//        cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, creation_003)
    {
        //003. if score not empty but no objects in firts instr/staff points to first
        //     instrument, first staff, timepos 0, measure 0, at end of staff.
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (staves 2)(musicData "
            "(clef F4 p2)(n e3 q v2 p2)"
            ")))" );
        m_pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(&doc, m_pScore);
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 0, 0.0f);
        //cout << cursor.dump_cursor();
    }

    //----------------------------------------------------------------------------
    // point_to()
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_101)
    {
        //101. point to an object, by object ID, does it ok
        create_document_1();
//        dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(27L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);
//        cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_102)
    {
        //102. point to an object, by object ID. Second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to(42L);

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 96.0, 42L, 42L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_103)
    {
        //103. point to an non-existing object, by object ID, moves to end of score
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(57L);
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 192.0);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_104)
    {
        //104. when at end, point to an object by object ID does it ok
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(57L);   //move to end (invalid object)
        cursor.point_to(27L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, point_to_105)
    {
        //105. point to a barline, second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to_barline(51L, 1);   //to barline, second staff

        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 256.0, 51L, 51L);
        //cout << cursor.dump_cursor();
    }

    //----------------------------------------------------------------------------
    // move_next()
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_151)
    {
        //151. single voice. move in sequence to next
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_152)
    {
        //152. single voice. move in sequence until end of score
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_next();
//        cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_rest() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( cursor.my_is_pointing_barline() == true );
        CHECK( (*cursor)->is_barline() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, 29L, 29L);

        cursor.move_next();
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 192.0);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_153)
    {
        //153. if at end of score, remains there
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(29L);       //move to last note

        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 192.0);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 192.0);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_154)
    {
        //154. many voices: move in sequence to next in current voice, no gaps
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 30L, 30L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 31L, 31L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0, 32L, 32L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 33L, 33L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 160.0, 34L, 34L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 35L, 35L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 256.0, 36L, 36L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 38L, 38L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 320.0, 39L, 39L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 352.0);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_155)
    {
        //155. many voices: move in sequence to next in current voice, with gaps
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(1);
        CHECK( (*cursor)->is_clef() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_time_signature() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK( (*cursor)->is_note() == true );
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);
        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 29L, 29L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 224.0, k_cursor_at_empty_place, 36L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 256.0, 36L, 36L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 37L, 37L);

        cursor.move_next();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 288.0);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_161)
    {
        //161. skips notes in chord: simple note after chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(true) << endl;
//        dump_col_staff_objs();
        cursor.point_to(27L);   //first note in chord

        cursor.move_next();

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 33L, 33L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_162)
    {
        //162. skips notes in chord: chord after chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(35L);   //first note in chord

        cursor.move_next();

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 42L, 42L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_163)
    {
        //163. skips notes in chord: end of score after chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(42L);   //first note in chord

        cursor.move_next();

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 256.0);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_171)
    {
        //171. to next object. must skip objects in other staves/instruments
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(true) << endl;
//        dump_col_staff_objs();
        cursor.point_to(27L);   //first note first staff

        cursor.move_next();

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 28L, 28L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_172)
    {
        //172. cross barline
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(42L);   //last note first measure second staff

        cursor.move_next();     //to barline
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 128.0, 48L, 48L);

        cursor.move_next();     //to first note in second measure second staff
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0, 49L, 49L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_173)
    {
        //173. cross last barline: move to end of staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //to last barline
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 51L, 51L);

        cursor.move_next();     //to end of staff

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0);
        CHECK( cursor.is_at_end_of_score() == false );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_174)
    {
        //174. from end of staff to next staff in same instrument
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //to barline
        cursor.move_next();     //to end of staff

        cursor.move_next();     //to clef F4 on second staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_175)
    {
        //175. from end of staff to next instrument
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(true) << endl;
//        dump_col_staff_objs();

        cursor.point_to(52L);   //to last note instr 1, staff 2
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0, 52L, 52L);

        cursor.move_next();     //to barline 52L
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 256.0, 53L, 53L);

        cursor.move_next();     //to end of instr 1, staff 2
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );

        cursor.move_next();     //to clef F4 on instr 2, staff 1
//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 1, 0, 0, 0.0, 56L, 56L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_176)
    {
        //176. at end of last staff, last instr. remains there (=153)
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(true) << endl;
//        dump_col_staff_objs();
        cursor.point_to(84L);   //to last note instr 2, staff 2
        cursor.move_next();     //to barline 84L

        cursor.move_next();     //to end of score

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 1, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.move_next();     //remains at end

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 1, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_177)
    {
        //177. traversing a sequence of notes to end of score (=152)
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cout << m_pDoc->to_string(true) << endl;
//        dump_col_staff_objs();

        cursor.point_to(61L);   //first note in instr 2, staff 2
        cursor.set_current_voice(2);
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 0.0, 61L, 61L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 16.0, 62L, 62L);
//        cout << cursor.dump_cursor();

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 32.0, 63L, 63L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 48.0, 64L, 64L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 64.0, 74L, 74L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 96.0, 75L, 75L);

        cursor.move_next();     //to barline 80L
        CHECK_CURRENT_STATE(cursor, 1, 1, 0, 128.0, 81L, 81L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 128.0, 83L, 83L);

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 192.0, 84L, 84L);

        cursor.move_next();     //to barline 84L
        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 256.0, 85L, 85L);

        cursor.move_next();     //to end of score
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 1, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_178)
    {
        //178. last note. move forward to end of staff
        create_document_3();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(50L);   //last note staff 2

        cursor.move_next();     //to end of staff

//        cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 1, 1, 256.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_179)
    {
        //179. to next object at same time: clef -> key -> time -> note
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef in staff 1

        cursor.move_next();     //key
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_next();     //time
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_next();     //note
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 27L, 27L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_180)
    {
        //180. skip implicit shapes: clef -> (skip key, time) -> note
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef in staff 2
        cursor.set_current_voice(2);
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);

        cursor.move_next();     //note (skips key & time)

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 34L, 34L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_181)
    {
        //181. from barline to start of next measure (empty)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(48L);   //barline
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 48L, 48L);

        cursor.move_next();

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, k_cursor_at_empty_place, 49L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_182)
    {
        //182. from note to empty pos. after the note
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);   //second note first staff
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 28L, 28L);

        cursor.move_next();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_183)
    {
        //183. from empty place (after note) to object (next barline)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);   //second note first staff
        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);

        cursor.move_next();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 48L, 48L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_184)
    {
        //184. from empty place (start of measure) to object (next barline)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(48L);   //barline
        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, k_cursor_at_empty_place, 49L);

        cursor.move_next();

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 51L, 51L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_185)
    {
        //185. bug1: from end of staff to next instrument
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(53L);   //last barline, instr.1, stf.1
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 53L, 53L);
        cursor.move_next();     //to end of instr.0, stf.0
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );

        cursor.move_next();     //to clef F4 on instr.1, stf.2

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_next_186)
    {
        //186. bug. only prolog, from clef to end of score, skipping implicit shapes
        create_document_9();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef F4, second staff
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();

        cursor.move_next();     //to end of score

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 1, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();
    }

    //----------------------------------------------------------------------------
    // move_prev
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_201)
    {
        //201. single voice: move back in sequence
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(29L);   //last note

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_202)
    {
        //202. single voice: move back in sequence until start of score
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(29L);   //last note
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, 29L, 29L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
        //CHECK( cursor.is_   = true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_203)
    {
        //203. if at start of score, remains there
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_204)
    {
        //204. many voices: move back in sequence in current voice, no gaps
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(39L);   //last note voice 2
        ++cursor;               //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 288.0);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.set_current_voice(2);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 320.0, 39L, 39L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 38L, 38L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 256.0, 36L, 36L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 35L, 35L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 160.0, 34L, 34L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 33L, 33L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0, 32L, 32L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 31L, 31L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 30L, 30L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_205)
    {
        //205. many voices: move back in sequence in current voice, with gaps
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(39L);   //last note in voice 2
        ++cursor;               //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 352.0);
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();

        cursor.change_voice_to(1);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 288.0);
        //cout << cursor.dump_cursor();

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 37L, 37L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 256.0, 36L, 36L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 224.0, k_cursor_at_empty_place, 36L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 29L, 29L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_prev();
        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_206)
    {
        //206. bug: only clef, at end.
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)(instrument (musicData (clef G))))))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef
        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_211)
    {
        //211. skips notes in chord: simple note before chord moves to first in chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(33L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 27L, 27L);
        //cout << cursor.dump_cursor();

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_212)
    {
        //212. skips notes in chord: chord before chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(46L);   //top note in last chord
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 46L, 46L);
        //cout << cursor.dump_cursor();

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 35L, 35L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_213)
    {
        //213. skips notes in chord: start of score before chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(31L);   //top note in first chord
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 31L, 31L);
        //cout << cursor.dump_cursor();

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_221)
    {
        //221. to prev object. must skip objects in other instruments
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(42L);   //last note staff 2, measure 1

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 64.0, 41L, 41L);

        cursor.move_prev();     //must skip note in staff 1
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 32.0, 35L, 35L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_222)
    {
        //222. back from object in second staff to barline. Continues in second staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(49L);   //note after barline, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0, 49L, 49L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 128.0, 48L, 48L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_223)
    {
        //223. from start of staff to end of previous staff in same instrument (barline)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef in instrument 1, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);

        cursor.move_prev();

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0);
        CHECK( cursor.is_at_end_of_score() == false );
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_224)
    {
        //224. from start of staff to end of previous staff in same instrument (note)
        create_document_3();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef in instrument 1, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);

        cursor.move_prev();

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 1, 128.0);
        CHECK( cursor.is_at_end_of_score() == false );
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_225)
    {
        //225. from end of staff to object
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef in instrument 1, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);

        cursor.move_prev();
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 51L, 51L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_226)
    {
        //226. from end of staff to chord
        create_document_6();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(46L);   //last note
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 46L, 46L);
        cursor.move_next();
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 256.0);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 42L, 42L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_227)
    {
        //227. from barline to start of measure (empty measure)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //barline
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 256.0, 51L, 51L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, k_cursor_at_empty_place, 49L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_228)
    {
        //228. to prev object at same time: note -> time --> key -> clef
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(27L);   //first note in instrument 1, staff 1
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 27L, 27L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 25L, 25L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_229)
    {
        //229. at start of score: remains there (=203)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef in instrument 1, staff 1
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.move_prev();     //to time. prev state is key signature

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_230)
    {
        //230. from start of staff to end of previous staff in prev instrument
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        //cout << m_pDoc->to_string(true) << endl;
        //dump_col_staff_objs();
        cursor.point_to(56L);   //clef instr 2, staff 1
        CHECK_CURRENT_STATE(cursor, 1, 0, 0, 0.0, 56L, 56L);

        cursor.move_prev();

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_231)
    {
        //231. skip implicit shapes: note -> (skip time & key) -> clef
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(34L);   //first note in instrument 1, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 34L, 34L);

        cursor.move_prev();     //to clef (skip time and key signatures)

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_232)
    {
        //232. back from object to empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(48L);   //first barline
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 48L, 48L);

        cursor.move_prev();     //after note in staff 1

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_233)
    {
        //233. back from empty place (start of measure) to object (prev. barline)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //barline
        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, k_cursor_at_empty_place, 49L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 48L, 48L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_234)
    {
        //234. back from empty place (after note) to object (the note)
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(48L);   //first barline
        cursor.move_prev();     //after note in staff 1
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 28L, 28L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_235)
    {
        //235. from start of staff -> to end of prev staff -> to barline
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(56L);   //start of instr 2, staff 1
        CHECK_CURRENT_STATE(cursor, 1, 0, 0, 0.0, 56L, 56L);

        cursor.move_prev();     //end of instr 1, staff 2
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);

        cursor.move_prev();     //barline of instr 1, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 256.0, 53L, 53L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_236)
    {
        //236. bug. moved to end of score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        m_pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(&doc, m_pScore);
        cursor.point_to(25L);   //last note
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 25L, 25L);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_237)
    {
        //237. bug. only prolog. move back when at end of staff
        create_document_9();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(26L);   //time signature
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);
        cursor.move_next();     //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 0, 0.0f);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 26L, 26L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, move_prev_238)
    {
        //238. bug. only prolog. back when at end of score
        create_document_9();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef F4, staff 2
        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        cursor.move_next();     //end of score
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 1, 0, 0.0f);

        cursor.move_prev();

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

    //----------------------------------------------------------------------------
    // other methods
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, iter_object_is_barline_301)
    {
        //301. iter_object_is_barline is true when pointing barline
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);       //move to barline

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);
        CHECK( (*cursor)->is_barline() == true );
        CHECK( cursor.my_is_pointing_barline() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, iter_object_is_barline_302)
    {
        //302. iter_object_is_barline is false when not pointing barline
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(27L);       //rest

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, 27L, 27L);
        CHECK( (*cursor)->is_rest() == true );
        CHECK( cursor.my_is_pointing_barline() == false );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_320)
    {
        //320. to measure, same instr, same staff
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.to_measure(1, -1, -1);

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, 44L, 44L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_321)
    {
        //321. to measure, same instr, higher staff
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);

        cursor.to_measure(1, -1, 1);

        CHECK_CURRENT_STATE(cursor, 0, 1, 1, 128.0, 52L, 52L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_322)
    {
        //322. to measure, same instr, lower staff
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);       //staff 1

        cursor.to_measure(1, -1, 0);

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, 44L, 44L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_323)
    {
        //323. to measure, other instr
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);

        cursor.to_measure(1, 1, 1);

        CHECK_CURRENT_STATE(cursor, 1, 1, 1, 128.0, 83L, 83L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_324)
    {
        //324. to measure, end of staff
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.to_measure(2, -1, -1);

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0);
        CHECK( cursor.is_at_end_of_score() == false );
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_325)
    {
        //325. to measure, at empty position
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.to_measure(1, -1, -1);

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 128.0, k_cursor_at_empty_place, 49L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_measure_326)
    {
        //326. to first measure
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(50L);       //measure 1, staff 1

        cursor.to_measure(0, -1, -1);

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

//    TEST_FIXTURE(ScoreCursorTestFixture, next_note_in_chord_340)
//    {
//        //340. to_next_note_in_chord()
//        create_document_6();
//        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(35L);
//    }
//
//    TEST_FIXTURE(ScoreCursorTestFixture, prev_note_in_chord_341)
//    {
//        //341. to_prev_note_in_chord()
//        create_document_6();
//        MyScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(35L);
//    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_350)
    {
        //350. cursor doesn't move when current pos. is empty for new voice.
        create_document_8();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(39L);
        cursor.set_current_voice(2);
        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 320.0, 39L, 39L);

        cursor.change_voice_to(1);

        CHECK_CURRENT_STATE(cursor, 0, 0, 1, 320.0, k_cursor_at_empty_place, 39L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_351)
    {
        //351. cursor doesn't move when current pos. is inside a gap for new voice.
        create_document_8();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(32L);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0, 32L, 32L);
        cursor.set_current_voice(2);

        cursor.change_voice_to(1);

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 96.0, k_cursor_at_empty_place, 32L);
        //cout << cursor.dump_cursor();

        cursor.move_next();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_352)
    {
        //352. object in new voice at same timepos: cursor moves to it.
        create_document_8();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 28L, 28L);

        cursor.change_voice_to(2);

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 128.0, 33L, 33L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_353)
    {
        //353. other cases: cursor moves to next valid position for new voice.
        create_document_8();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(34L);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 160.0, 34L, 34L);

        cursor.change_voice_to(1);

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 192.0, 29L, 29L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_354)
    {
        //354. end of score time depends on voice: move time back
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(39L);   //last note in voice 2
        ++cursor;               //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 352.0);
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();

        cursor.change_voice_to(1);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 288.0);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, change_active_voice_355)
    {
        //355. end of score time depends on voice: move time fwd.
        create_document_8();
        //dump_col_staff_objs();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.set_current_voice(2);
        cursor.point_to(39L);   //last note in voice 2
        ++cursor;               //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 352.0);
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();
        cursor.change_voice_to(1);
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 288.0);
        //cout << cursor.dump_cursor();

        cursor.change_voice_to(2);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 1, 352.0);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, to_time_360)
    {
        //360. to start of second staff
        create_document_9();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(24L);   //clef F4 in staff 2
        ++cursor;               //end of staff
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 1, 0, 0.0);
        CHECK( cursor.is_at_end_of_score() == true );

        cursor.to_time(0, 1, 0.0);

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }

////    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_1)
////    {
////        //1. skip objects
////        Document doc(m_libraryScope);
////        doc.from_file(m_scores_path + "01013-two-instruments-four-staves.lms" );
////        DocCursor cursor(m_pDoc);
////        cursor.enter_element();
////        cursor.point_to(24L);
////        cursor.skip_clef_key_time();
////        //cout << cursor.dump_cursor();
////        CHECK( cursor.id() == 28L );
////        CHECK( cursor.instrument() == 0 );
////        CHECK( cursor.measure() == 0 );
////        CHECK( cursor.staff() == 0 );
////        CHECK( is_equal_time(cursor.time(), 0.0f) );
////    }
////
////    TEST_FIXTURE(ScoreCursorTestFixture, ScoreCursorSkipClefKeyTime_2)
////    {
////        //2. nothing to skip: remains at n7
////        Document doc(m_libraryScope);
////        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q))))))" );
////        DocCursor cursor(m_pDoc);
////        cursor.enter_element();
////        cursor.point_to(9L);
////        cursor.skip_clef_key_time();
////        //cout << cursor.dump_cursor();
////        CHECK( cursor.id() == 9L );
////        CHECK( cursor.instrument() == 0 );
////        CHECK( cursor.measure() == 0 );
////        CHECK( cursor.staff() == 0 );
////        CHECK( is_equal_time(cursor.time(), 0.0f) );
////    }


    //----------------------------------------------------------------------------
    // get / restore state
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_401)
    {
        //401. get state: pointing object
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(52L);   //first half note, instr 1, staff 2, measure 2

        SpElementCursorState spState = cursor.get_state();

        ScoreCursorState* state = static_cast<ScoreCursorState*>( spState.get() );
        CHECK_STATE(state, 0, 1, 1, 128.0, 52L, 52L);
        //dump_cursor_state(state);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_402)
    {
        //402. get state: pointing to end of score
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //to barline
        cursor.move_next();     //to end of staff

        SpElementCursorState spState = cursor.get_state();

        ScoreCursorState* state = static_cast<ScoreCursorState*>( spState.get() );
        CHECK_STATE(state, 0, 0, 2, 256.0, k_cursor_at_end_of_staff, -1L);
        //dump_cursor_state(state);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_403)
    {
        //403. get state: pointing to empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);   //second note firt staff
        cursor.move_next();     //empty place

        SpElementCursorState spState = cursor.get_state();

        ScoreCursorState* state = static_cast<ScoreCursorState*>( spState.get() );
        CHECK_STATE(state, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);
        //dump_cursor_state(state);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_404)
    {
        //404. restore state: pointing object
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(42L);   //last note in staff 2, measure 1
        SpElementCursorState spState = cursor.get_state();
        cursor.point_to(23L);   //clef in staff 1

        cursor.restore_state(spState);

        CHECK_CURRENT_STATE(cursor, 0, 1, 0, 96.0, 42L, 42L);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_405)
    {
        //405. restore state: pointing to end of staff
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(51L);   //to barline
        cursor.move_next();     //to end of staff
        SpElementCursorState spState = cursor.get_state();
        CHECK( cursor.is_at_end_of_score() == false );
        cursor.point_to(23L);   //clef in staff 1

        cursor.restore_state(spState);

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 0, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == false );
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_406)
    {
        //406. restore state: pointing to empty place
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(28L);   //second note firt staff
        cursor.move_next();     //to empty place
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);
        SpElementCursorState spState = cursor.get_state();
        cursor.point_to(51L);   //to barline

        cursor.restore_state(spState);

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 64.0, k_cursor_at_empty_place, 41L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_407)
    {
        //407. restore state: pointing to end of score
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to_barline(51L, 1);   //to barline, second staff
        cursor.move_next();     //to end of score
        CHECK( cursor.is_at_end_of_score() == true );
        SpElementCursorState spState = cursor.get_state();
        cursor.point_to(23L);   //clef in staff 1

        cursor.restore_state(spState);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 1, 2, 256.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_408)
    {
        //408. bug1. restore end of staff state: bad instrument
        create_document_7();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(56L);   //start of instr 2, staff 1
        cursor.move_prev();     //end of instr 1, staff 2
        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        SpElementCursorState spState = cursor.get_state();
        cursor.move_prev();     //barline of instr 1, staff 2

        cursor.restore_state(spState);

        CHECK_CURRENT_STATE_AT_END_OF_STAFF(cursor, 0, 1, 2, 256.0f);
        //cout << cursor.dump_cursor();
    }

    TEST_FIXTURE(ScoreCursorTestFixture, get_restore_state_409)
    {
        //409. bug. moved to end of score
        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0)(instrument (musicData "
            "(clef G)(n e4 e g+)(n g4 s g-)"
            ")))");
        m_pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(&doc, m_pScore);
        cursor.point_to(25L);   //last note
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 25L, 25L);
        SpElementCursorState spState = cursor.get_state();
        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);

        cursor.restore_state(spState);
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 32.0, 25L, 25L);

        cursor.move_prev();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 24L, 24L);
        //cout << cursor.dump_cursor();
    }


    //----------------------------------------------------------------------------
    // integrity preservation after score modifications
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_501)
    {
        //501. reset_and_point_to
        create_document_empty_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );

        ImoClef* pImo = static_cast<ImoClef*>( ImFactory::inject(k_imo_clef, m_pDoc) );
        ImoId id = pImo->get_id();
        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->insert_staffobj_at(NULL /*at start*/, pImo);
        m_pScore->close();
        //dump_col_staff_objs();

        cursor.reset_and_point_to(id);

        //cout << cursor.dump_cursor();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, id, id);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_510)
    {
        //510. reset and point after: previous at end_of_score
        create_document_empty_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
        ImoId idPrev = cursor.get_pointee_id();

        cursor.reset_and_point_after(idPrev);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_511)
    {
        //511. reset and point after: previous before_start_of_child
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)(instrument (musicData (clef G))))))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
        SpElementCursorState spState = cursor.find_previous_pos_state();
        ImoId idPrev = spState->pointee_id();
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);
        ImoClef* pImo = static_cast<ImoClef*>( cursor.get_pointee() );

        ImoInstrument* pInstr = m_pScore->get_instrument(0);
        pInstr->delete_staffobj(pImo);
        m_pScore->close();

        cursor.reset_and_point_after(idPrev);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_512)
    {
        //512. reset and point after: previous an Imo and next Imo exists
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)(instrument (musicData (clef G)(key C))))))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.reset_and_point_after(23L);

        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0f, 24L, 24L);
    }

    TEST_FIXTURE(ScoreCursorTestFixture, survives_updates_513)
    {
        //513. reset and point after: previous an Imo and next is end of score
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0)(instrument (musicData (clef G))))))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef
        CHECK_CURRENT_STATE(cursor, 0, 0, 0, 0.0, 23L, 23L);

        cursor.reset_and_point_after(23L);

        CHECK_CURRENT_STATE_AT_END_OF_SCORE(cursor, 0, 0, 0, 0.0f);
        CHECK( cursor.is_at_end_of_score() == true );
    }


    //----------------------------------------------------------------------------
    // Cursor collects information for computing time and timecode
    //----------------------------------------------------------------------------

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_601)
    {
        //601. if score empty, duration == 0, timepos == 0
        create_document_empty_score();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        TimeInfo ti = cursor.get_time_info();

        CHECK( ti.get_timepos() == 0.0 );
        CHECK( ti.get_score_total_duration() == 0.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_602)
    {
        //602. if score not empty, duration >= 0, timepos == 0
        create_document_1();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        TimeInfo ti = cursor.get_time_info();

        CHECK( ti.get_timepos() == 0.0 );
        CHECK( ti.get_score_total_duration() == 192.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_610)
    {
        //610. point_to() updates time related info
        create_document_5();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to(48L);   //2nd note in 2nd bar

        TimeInfo ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 224.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter_dotted );
        CHECK( ti.get_current_measure_start_timepos() == 192.0 );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_611)
    {
        //611. point_to_barline() updates time related info
        create_document_5();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to_barline(46L, 0);   //1st barline

        TimeInfo ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 192.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter_dotted );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_612)
    {
        //612. move_next() updates time related info
        create_document_5();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(23L);   //clef in staff 1

        cursor.move_next();     //key
        CHECK( cursor.get_time_info().get_current_beat_duration() == k_duration_quarter );

        cursor.move_next();     //time
        CHECK( cursor.get_time_info().get_current_beat_duration() == k_duration_quarter );

        cursor.move_next();     //note
        CHECK( cursor.get_time_info().get_current_beat_duration() == k_duration_quarter_dotted );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_613)
    {
        //613. move_prev() updates time related info
        create_document_5();
        MyScoreCursor cursor(m_pDoc, m_pScore);

        cursor.point_to(47L);   //1st note in 2nd bar

        TimeInfo ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 192.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter_dotted );
        CHECK( ti.get_current_measure_start_timepos() == 192.0 );

        cursor.move_prev();     //barline
        ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 192.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter_dotted );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );

        cursor.move_prev();     //note
        cursor.move_prev();     //note
        cursor.move_prev();     //note
        cursor.move_prev();     //note
        cursor.move_prev();     //note
        cursor.move_prev();     //note
        ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 0.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter_dotted );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );

        cursor.move_prev();     //time signature
        ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 0.0 );
        CHECK( ti.get_score_total_duration() == 384.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );
    }

    TEST_FIXTURE(ScoreCursorTestFixture, time_info_614)
    {
        //614. restore_state() updates time related info
        create_document_2();
        MyScoreCursor cursor(m_pDoc, m_pScore);
        cursor.point_to(42L);   //last note in staff 2, measure 1
        SpElementCursorState spState = cursor.get_state();
        cursor.point_to(23L);   //clef in staff 1

        cursor.restore_state(spState);

        TimeInfo ti = cursor.get_time_info();
        CHECK( ti.get_timepos() == 96.0 );
        CHECK( ti.get_score_total_duration() == 256.0 );
        CHECK( ti.get_current_beat_duration() == k_duration_quarter );
        CHECK( ti.get_current_measure_start_timepos() == 0.0 );
    }

};

//=======================================================================================
class TimeInfoTestFixture
{
public:
    LibraryScope m_libraryScope;

    TimeInfoTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~TimeInfoTestFixture()    //TearDown fixture
    {
    }
};

SUITE(TimeInfoTest)
{

    TEST_FIXTURE(TimeInfoTestFixture, timecode_1)
    {
        TimeUnits curTimepos = 230.0;
        TimeUnits totalDuration = 1280.0;
        TimeUnits curBeatDuration = 64.0;
        TimeUnits startOfBarTimepos = 128.0;
        int measure = 1;
        TimeInfo ti(curTimepos, totalDuration, curBeatDuration, startOfBarTimepos, measure);

        Timecode tc = ti.get_timecode();

        CHECK( tc.bar == 2 );
        CHECK( tc.beat == 2 );
        CHECK( tc.n16th == 2 );
        CHECK( tc.ticks == 45 );
    }

    TEST_FIXTURE(TimeInfoTestFixture, timecode_2)
    {
        TimeUnits curTimepos = 256.0;
        TimeUnits totalDuration = 1280.0;
        TimeUnits curBeatDuration = 64.0;
        TimeUnits startOfBarTimepos = 128.0;
        int measure = 1;
        TimeInfo ti(curTimepos, totalDuration, curBeatDuration, startOfBarTimepos, measure);

        CHECK( ti.played_percentage() == 20.0f );
        CHECK( ti.remaining_percentage() == 80.0f );
    }

    TEST_FIXTURE(TimeInfoTestFixture, timecode_3)
    {
        //at start of empty score
        TimeUnits curTimepos = 0.0;
        TimeUnits totalDuration = 0.0;
        TimeUnits curBeatDuration = 64.0;
        TimeUnits startOfBarTimepos = 0.0;
        int measure = 0;
        TimeInfo ti(curTimepos, totalDuration, curBeatDuration, startOfBarTimepos, measure);

        CHECK( ti.played_percentage() == 100.0f );
        CHECK( ti.remaining_percentage() == 0.0f );
    }

};

