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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_ldp_factory.h"
#include "lomse_document.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_score_layouter.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_im_note.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class ColumnBreakerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ColumnBreakerTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~ColumnBreakerTestFixture()    //TearDown fixture
    {
    }

    void find_next_break(ColumnBreaker& breaker, StaffObjsCursor& soCursor)
    {
        while(!soCursor.is_end() )
        {
            ImoStaffObj* pSO = soCursor.get_staffobj();
            int iInstr = soCursor.num_instrument();
//            int iStaff = soCursor.staff();
            int iLine = soCursor.line();
            TimeUnits rTime = soCursor.time();
            //TimeUnits rNextTime = soCursor.next_staffobj_timepos();

            if ( breaker.feasible_break_before_this_obj(pSO, rTime, iInstr, iLine) )
                break;

            soCursor.move_next();
        }
    }

};

SUITE(ColumnBreakerTest)
{

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_101)
    {
        //@ 101. one staff, with time signature: break after barlines

        Document doc(m_libraryScope);
        //"(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)(barline#27L)"
        //"(n#28L d4 q)(n#29L g4 q)(barline#30L)"
        //"(n#31L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_101" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 28L );
        CHECK( is_equal_time(soCursor.time(), 128.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 31L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_102)
    {
        //@ 102. one staff, with time signature: don't break after middle barlines

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 4 4)(n#25L c4 q)(n#26L e4 q)
        //(barline#27L simple middle)
        //(n#28L d4 q)(n#29L g4 q)(barline#30L)"
        //(n#31L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 4 4)(n c4 q)(n e4 q)"
            "(barline simple middle)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_102" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 31L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_111)
    {
        //@ 111. one staff with TS, measure duration exceeded:
        //@      At next barline if just one staffobj exceeded

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)(n#27L g4 q)
        //(barline#28L)
        //(n#29L d4 q)(n#30L g4 q)(barline#31L)"
        //(n#32L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_111" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 29L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 32L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_112)
    {
        //@ 112. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Isolated notes

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)(n#27L g4 q)
        //(n#28L c5 q)(barline#29L)
        //(n#30L d4 q)(n#31L g4 q)(barline#32L)"
        //(n#33L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)(n c5 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_112" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 28L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 30L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 33L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_113)
    {
        //@ 113. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Beamed notes

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 e)(n#27L g4 e)
        //(n#28L c5 q)(barline#34L)
        //(n#35L d4 q)(n#36L g4 q)(barline#37L)"
        //(n#38L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 e g+)(n c5 e g-)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_113" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 35L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 38L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_114)
    {
        //@ 114. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Tied prev notes

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)(n#27L g4 e l+)
        //(n#28L g4 e)(barline#34L)
        //(n#35L d4 q)(n#36L g4 q)(barline#37L)"
        //(n#38L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 e l+)(n g4 e)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_114" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 35L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 38L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_115)
    {
        //@ 115. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Rests

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)(n#27L g4 q)
        //(r#28L q)(barline#29L)
        //(n#30L d4 q)(n#31L g4 q)(barline#32L)"
        //(n#33L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)(r q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_115" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 28L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 30L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 33L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_116)
    {
        //@ 116. one staff with TS, measure duration exceeded:
        //@ after middle barlines

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)
        //(n#27L g4 q)(barline#28L simple middle)
        //(n#29L c5 q)(barline#30L)
        //(n#31L d4 q)(n#32L g4 q)(barline#33L)"
        //(n#34L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 q)(barline simple middle)(n c5 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_116" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 29L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 31L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 34L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_131)
    {
        //@ 131. Last measure open:
        //@      Before suitable notes/rests once measure duration exceeded

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(time#24L 2 4)(n#25L c4 q)(n#26L e4 q)
        //(barline#27L)
        //(n#28L g4 q)(n#29L c5 q)(n#30L e4 e l+)
        //(n#31L e4 q)(n#37L g4 e g+)(n#38L e4 e g-)(n#44L b4 q))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n g4 q)(n c5 q)(n e4 e l+)"
            "(n e4 q)(n g4 e g+)(n e4 e g-)(n b4 q)))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_131" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 28L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 37L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 44L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_201)
    {
        //@ 201. one staff scores, no time signature:
        //@      Before suitable notes/rests

        Document doc(m_libraryScope);
        //(musicData (clef#23L G)(n#24L c4 q)(n#25L e4 q)
        //(n#26L g4 q)(n#27L c5 q)(n#28L e4 e l+)
        //(n#29L e4 q)(n#35L g4 e g+)(n#36L e4 e g-)(n#42L b4 q))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(n e4 q)"
            "(n g4 q)(n c5 q)(n e4 e l+)"
            "(n e4 q)(n g4 e g+)(n e4 e g-)(n b4 q)))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_201" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 25L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 26L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 27L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 28L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 35L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 42L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_301)
    {
        //@ 301. one instrument, multi-voice:
        //@      Break only before objects that imply a new timepos

        Document doc(m_libraryScope);
            //(clef#23L G p1)(clef#24L F4 p2)(time#25L 2 4)"
            //(n#26L c4 q p1 v1)(n#27L e4 q)
            //(n#28L c3 q p2 v2)(n#29L e3 q)(barline#30L)"
            //(n#31L g4 q p1 v1)(n#32L c5 q)(n#33L e5 q)(n#34L g5 q)(n#35L c6 q)
            //(n#36L e3 q p2 v2)(n#37L c4 q)(n#38L e4 q)(n#39L g4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2) (musicData "
            "(clef G p1)(clef F4 p2)(time 2 4)"
            "(n c4 q p1 v1)(n e4 q)"
            "(n c3 q p2 v2)(n e3 q)(barline)"
            "(n g4 q p1 v1)(n c5 q)(n e5 q)(n g5 q)(n c6 q)"
            "(n e3 q p2 v2)(n c4 q)(n e4 q)(n g4 q) ))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_301" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 31L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 34L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 35L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_302)
    {
        //@ 302. one instrument, multi-voice:
        //@      Do not split a note/rest in other voice

        Document doc(m_libraryScope);
            //(clef#23L G p1)(clef#24L F4 p2)(time#25L 2 4)"
            //(n#26L c4 q p1 v1)(n#27L e4 q)
            //(n#28L c3 q p2 v2)(n#29L e3 q)(barline#30L)"
            //(n#31L g4 q p1 v1)(n#32L c5 q)(n#33L e5 q)(n#34L g5 q)
            //(n#35L c6 q)(n#36L b5 q)(n#37L a5 q)
            //(n#38L e3 w p2 v2)(n#39L f3 h)"
            //(n#40L a2 q p2 v3)(n#41L c3 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2) (musicData "
            "(clef G p1)(clef F4 p2)(time 2 4)"
            "(n c4 q p1 v1)(n e4 q)"
            "(n c3 q p2 v2)(n e3 q)(barline)"
            "(n g4 q p1 v1)(n c5 q)(n e5 q)(n g5 q)"
            "(n c6 q)(n b5 q)(n a5 q)"
            "(n e3 w p2 v2)(n f3 h)"
            "(n a2 q p2 v3)(n c3 q) ))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_302" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 31L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 35L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 37L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_303)
    {
        //@ 303. one instrument, multi-voice:
        //@      Do not break beams in other voice

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 e p1 v1 g+)(n d4 s p1)(n e4 s p1 g-)(n f4 s p1 g+)(n g4 s p1 g-)"
            "(n c4 s p1 g+)(n d4 s g-)(n e4 q)"
            "(n c3 q p2 v2)(n d3 s p2 g+)(n e3 s)(n f3 s)(n g3 s g-)"
            ")) )))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_302" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 36L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 50L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_304)
    {
        //@ 304. one instrument, multi-voice:
        //@      Do not break ties in other voice

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 e p1 v1 g+)(n d4 s p1)(n e4 s p1 g-)(n f4 s p1 g+)(n g4 s p1 g-)"
            "(n c4 s p1 g+)(n d4 s g-)(n e4 q)"
            "(n c3 q p2 v2)(n d3 e l+)(n d3 e)"
            ")) )))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_302" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 36L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 50L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_401)
    {
        //@ 401. in multi-instrument, no TS
        //@      Do not break just before a barline

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key C)"
            "(n c4 e g+)(n d4 e g-)(barline)(n e4 q) ))"
            "(instrument (musicData (clef F4)(key C)"
            "(n c3 q)(barline)(n d3 q) ))"
            ")))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << "test: column_breaker_401" << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 33L );
        CHECK( is_equal_time(soCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_failure_5)
    {
        //@ test a multimetric score

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData "
            "(clef G)(key G)(time 3 4)(chord (n g3 q)(n d4 q))"
            "(r e)(n g5 e)(n g5 s g+)(n f5 s)(n g5 e g-)(barline)"
            "(r h.)(barline)"
            "(chord (n g3 h)(n d4 h))(chord (n g3 q)(n d4 q))(barline)"
            "(chord (n a4 q)(n e5 q))(r q)"
            "(chord (n d4 q)(n g4 q)(n f5 q))(barline) ))"
            "(instrument (musicData "
            "(clef G)(key G)(time 2 4)"
            "(n g4 q)(n d5 e g+)(n d5 e g-)(barline)"
            "(n b5 e g+)(n a5 s)(n g5 s g-)"
            "(n g5 e g+)(n g5 e g-)(barline)"
            "(n e5 e g+)(n d5 s)(n c5 s g-)"
            "(n e5 e g+)(n e5 e g-)(barline)"
            "(n d5 e g+)(n g5 e)(n g5 e)(n g5 e g-)(barline)"
            "(n b5 e g+)(n g5 e g-)"
            "(n e5 s g+)(n c5 s)(n e5 s)(n c5 s g-)(barline)"
            "(n b4 s g+)(n d5 s)(n b4 s)(n d5 s g-)"
            "(n c5 e g+)(n b4 s)(n a4 s g-)(barline) ))"
            ")))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);
        //cout << pScore->get_staffobjs_table()->dump() << endl;

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);
        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 47L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

////    TEST_FIXTURE(ColumnBreakerTestFixture, DumpStaffobsTable)
////    {
////        //@ auxiliary, for dumping table in scores with problems
////
////        Document doc(m_libraryScope);
////        doc.from_file(m_scores_path + "00201-systems-are-justified.lms");
////        ImoScore* pScore = static_cast<ImoScore*>( doc.get_imodoc()->get_content_item(0) );
////        cout << pScore->get_staffobjs_table()->dump() << endl;
////    }

}


