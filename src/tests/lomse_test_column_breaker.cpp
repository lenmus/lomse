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
#include "private/lomse_document_p.h"
#include "lomse_staffobjs_table.h"
#include "lomse_internal_model.h"
#include "lomse_score_layouter.h"
#include "lomse_staffobjs_cursor.h"
#include "lomse_im_note.h"
#include "lomse_time.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;

//---------------------------------------------------------------------------------------
// helper, for accessing protected members
class MyColumnBreaker : public ColumnBreaker
{
public:
    MyColumnBreaker(int numInstruments, StaffObjsCursor* pSysCursor)
        : ColumnBreaker(numInstruments, pSysCursor)
    {
    }
    ~MyColumnBreaker() {}


    inline int is_in_barlines_mode() { return m_breakMode == ColumnBreaker::k_barlines; }
    inline int is_in_clear_cuts_mode() { return m_breakMode == ColumnBreaker::k_clear_cuts; }
    inline TimeUnits measure_mean_time() { return m_measureMeanTime; }

};


//---------------------------------------------------------------------------------------
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

    bool exec_feasible_break_before_this_obj(ColumnBreaker& breaker, StaffObjsCursor& soCursor)
    {
        ImoStaffObj* pSO = soCursor.get_staffobj();
        int iInstr = soCursor.num_instrument();
        int iLine = soCursor.line();
        TimeUnits rTime = soCursor.time();
        return breaker.feasible_break_before_this_obj(pSO, rTime, iInstr, iLine);
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

SUITE(ColumnBreakerTest)
{

//    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_001)
//    {
//        //@ 001. initially in clear-cuts mode
//
//        Document doc(m_libraryScope);
//        //"(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(barline#127L)"
//        //"(n#128L d4 q)(n#129L g4 q)(barline#130L)"
//        //"(n#131L e4 q) ))) ))" );
//        doc.from_string("(score (vers 2.0) "
//            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
//            "(n d4 q)(n g4 q)(barline)"
//            "(n e4 q) )))" );
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        StaffObjsCursor soCursor(pScore);
//
//        MyColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
//
//        CHECK( breaker.is_in_clear_cuts_mode() );
//    }
//
//    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_002)
//    {
//        //@ 002. As soon as TS detected in all instruments change to barlines mode
//
//        Document doc(m_libraryScope);
//        //"(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(barline#127L)"
//        //"(n#128L d4 q)(n#129L g4 q)(barline#130L)"
//        //"(n#131L e4 q) ))) ))" );
//        doc.from_string("(score (vers 2.0) "
//            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
//            "(n d4 q)(n g4 q)(barline)"
//            "(n e4 q) )))" );
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        StaffObjsCursor soCursor(pScore);
//
//        MyColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
//        //here soCursor points to clef
//        exec_feasible_break_before_this_obj(breaker, soCursor);
//        CHECK( breaker.is_in_clear_cuts_mode() );
//
//        soCursor.move_next();   //->time
//        exec_feasible_break_before_this_obj(breaker, soCursor);
//        CHECK( breaker.is_in_barlines_mode() );
//        CHECK( is_equal_time(breaker.measure_mean_time(), 106.667f) );
////        cout << test_name() << endl;
////        cout << "Measure mean time = " << breaker.measure_mean_time() << endl;
////        cout << pScore->get_staffobjs_table()->dump() << endl;
//    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_003)
    {
        //@ 003. Barline mean time when score ends in barline

        Document doc(m_libraryScope);
        //"(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(barline#127L)"
        //"(n#128L d4 q)(n#129L g4 q)(barline#130L)"
        //"))) ))" );
        doc.from_string("(score (vers 2.0) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n d4 q)(n g4 q)(barline)"
            ")))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        MyColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        //here soCursor points to clef
        exec_feasible_break_before_this_obj(breaker, soCursor);

        soCursor.move_next();   //->time
        exec_feasible_break_before_this_obj(breaker, soCursor);
        CHECK( is_equal_time(breaker.measure_mean_time(), 128.0f) );
//        cout << test_name() << endl;
//        cout << "Measure mean time = " << breaker.measure_mean_time()
//             << ", num. measures = " << soCursor.num_measures() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_004)
    {
        //@ 004. In barline mode. When duration exceeded move to clear-cuts mode

        Document doc(m_libraryScope);
        doc.from_string("(score (vers 2.0) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n d4 q)(n g4 q)(barline)"
            ")))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        MyColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        //here soCursor points to clef
        exec_feasible_break_before_this_obj(breaker, soCursor);

        soCursor.move_next();   //->time
        exec_feasible_break_before_this_obj(breaker, soCursor);
        soCursor.move_next();   //->n c4 q
        exec_feasible_break_before_this_obj(breaker, soCursor);
        soCursor.move_next();   //->n e4 q
        exec_feasible_break_before_this_obj(breaker, soCursor);
        soCursor.move_next();   //->n d4 q
        exec_feasible_break_before_this_obj(breaker, soCursor);
        CHECK( breaker.is_in_barlines_mode() );
        soCursor.move_next();   //->n g4 q
        exec_feasible_break_before_this_obj(breaker, soCursor);
        CHECK( breaker.is_in_clear_cuts_mode() );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_101)
    {
        //@ 101. one staff, with time signature: break after barlines

        Document doc(m_libraryScope);
        //"(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(barline#127L)"
        //"(n#128L d4 q)(n#129L g4 q)(barline#130L)"
        //"(n#131L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 128L );
        CHECK( is_equal_time(soCursor.time(), 128.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 131L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_102)
    {
        //@ 102. one staff, with time signature: don't break after middle barlines

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 4 4)(n#125L c4 q)(n#126L e4 q)
        //(barline#127L simple middle)
        //(n#128L d4 q)(n#129L g4 q)(barline#130L)"
        //(n#131L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 4 4)(n c4 q)(n e4 q)"
            "(barline simple middle)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 131L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_111)
    {
        //@ 111. one staff with TS, measure duration exceeded:
        //@      At next barline if just one staffobj exceeded

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(n#127L g4 q)
        //(barline#128L)
        //(n#129L d4 q)(n#130L g4 q)(barline#131L)"
        //(n#132L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 129L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_111a)
    {
        //@ 111a. one staff with TS, measure duration exceeded:
        //@      Breaker returns to barline mode after measure exceeded

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(n#127L g4 q)
        //(barline#128L)
        //(n#129L d4 q)(n#130L g4 q)(barline#131L)"
        //(n#132L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);
        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 132L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_112)
    {
        //@ 112. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Isolated notes

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(n#127L g4 q)
        //(n#128L c5 q)(barline#129L)
        //(n#130L d4 q)(n#131L g4 q)(barline#132L)"
        //(n#133L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)(n c5 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 128L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 130L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 133L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_113)
    {
        //@ 113. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Beamed notes

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 e)(n#127L g4 e)
        //(n#128L c5 q)(barline#134L)
        //(n#135L d4 q)(n#136L g4 q)(barline#137L)"
        //(n#138L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 e g+)(n c5 e g-)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 135L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 138L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_114)
    {
        //@ 114. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Tied prev notes

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(n#127L g4 e l+)
        //(n#128L g4 e)(barline#134L)
        //(n#135L d4 q)(n#136L g4 q)(barline#137L)"
        //(n#138L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 e l+)(n g4 e)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 135L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 138L );
        CHECK( is_equal_time(soCursor.time(), 320.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_115)
    {
        //@ 115. one staff with TS, measure duration exceeded:
        //@      Before suitable notes/rests. Rests

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)(n#127L g4 q)
        //(r#128L q)(barline#129L)
        //(n#130L d4 q)(n#131L g4 q)(barline#132L)"
        //(n#133L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(n g4 q)(r q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 128L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 130L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 133L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_116)
    {
        //@ 116. one staff with TS, measure duration exceeded:
        //@ after middle barlines

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)
        //(n#127L g4 q)(barline#128L simple middle)
        //(n#129L c5 q)(barline#130L)
        //(n#131L d4 q)(n#132L g4 q)(barline#133L)"
        //(n#134L e4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)"
            "(n g4 q)(barline simple middle)(n c5 q)"
            "(barline)(n d4 q)(n g4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 129L );
        CHECK( is_equal_time(soCursor.time(), 192.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 131L );
        CHECK( is_equal_time(soCursor.time(), 256.0f) );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 134L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_131)
    {
        //@ 131. Last measure open:
        //@      Before suitable notes/rests once measure duration exceeded

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(time#124L 2 4)(n#125L c4 q)(n#126L e4 q)
        //(barline#127L)
        //(n#128L g4 q)(n#129L c5 q)(n#130L e4 e l+)
        //(n#131L e4 q)(n#137L g4 e g+)(n#138L e4 e g-)(n#144L b4 q))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n g4 q)(n c5 q)(n e4 e l+)"
            "(n e4 q)(n g4 e g+)(n e4 e g-)(n b4 q)))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 128L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 137L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 144L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_201)
    {
        //@ 201. one staff scores, no time signature:
        //@      Before suitable notes/rests

        Document doc(m_libraryScope);
        //(musicData (clef#123L G)(n#124L c4 q)(n#125L e4 q)
        //(n#126L g4 q)(n#127L c5 q)(n#128L e4 e l+)
        //(n#129L e4 q)(n#135L g4 e g+)(n#136L e4 e g-)(n#142L b4 q))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(n c4 q)(n e4 q)"
            "(n g4 q)(n c5 q)(n e4 e l+)"
            "(n e4 q)(n g4 e g+)(n e4 e g-)(n b4 q)))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

        //cout << test_name() << endl;
        //cout << pScore->get_staffobjs_table()->dump() << endl;
        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 125L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 126L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 127L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 128L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 135L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 142L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_301)
    {
        //@ 301. one instrument, multi-voice:
        //@      Break only before objects that imply a new timepos

        Document doc(m_libraryScope);
            //(clef#123L G p1)(clef#124L F4 p2)(time#125L 2 4)"
            //(n#126L c4 q p1 v1)(n#127L e4 q)
            //(n#128L c3 q p2 v2)(n#129L e3 q)(barline#130L)"
            //(n#131L g4 q p1 v1)(n#132L c5 q)(n#133L e5 q)(n#134L g5 q)(n#135L c6 q)
            //(n#136L e3 q p2 v2)(n#137L c4 q)(n#138L e4 q)(n#139L g4 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument#121L (staves 2) (musicData "
            "(clef G p1)(clef F4 p2)(time 2 4)"
            "(n c4 q p1 v1)(n e4 q)"
            "(n c3 q p2 v2)(n e3 q)(barline)"
            "(n g4 q p1 v1)(n c5 q)(n e5 q)(n g5 q)(n c6 q)"
            "(n e3 q p2 v2)(n c4 q)(n e4 q)(n g4 q) ))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 131L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 134L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 135L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_302)
    {
        //@ 302. one instrument, multi-voice:
        //@      Do not split a note/rest in other voice

        Document doc(m_libraryScope);
            //(clef#123L G p1)(clef#124L F4 p2)(time#125L 2 4)"
            //(n#126L c4 q p1 v1)(n#127L e4 q)
            //(n#128L c3 q p2 v2)(n#129L e3 q)(barline#130L)"
            //(n#131L g4 q p1 v1)(n#132L c5 q)(n#133L e5 q)(n#134L g5 q)
            //(n#135L c6 q)(n#136L b5 q)(n#137L a5 q)
            //(n#138L e3 w p2 v2)(n#139L f3 h)"
            //(n#140L a2 q p2 v3)(n#141L c3 q) ))) ))" );
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument#121L (staves 2) (musicData "
            "(clef G p1)(clef F4 p2)(time 2 4)"
            "(n c4 q p1 v1)(n e4 q)"
            "(n c3 q p2 v2)(n e3 q)(barline)"
            "(n g4 q p1 v1)(n c5 q)(n e5 q)(n g5 q)"
            "(n c6 q)(n b5 q)(n a5 q)"
            "(n e3 w p2 v2)(n f3 h)"
            "(n a2 q p2 v3)(n c3 q) ))) ))" );
        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 131L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 135L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 137L );

    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_303)
    {
        //@ 303. one instrument, multi-voice:
        //@      Do not break beams in other voice

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument#121L (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 e p1 v1 g+)(n d4 s p1)(n e4 s p1 g-)(n f4 s p1 g+)(n g4 s p1 g-)"
            "(n c4 s p1 g+)(n d4 s g-)(n e4 q)"
            "(n c3 q p2 v2)(n d3 s p2 g+)(n e3 s)(n f3 s)(n g3 s g-)"
            ")) )))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 136L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 150L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_304)
    {
        //@ 304. one instrument, multi-voice:
        //@      Do not break ties in other voice

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 2.0) "
            "(instrument#121L (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 e p1 v1 g+)(n d4 s p1)(n e4 s p1 g-)(n f4 s p1 g+)(n g4 s p1 g-)"
            "(n c4 s p1 g+)(n d4 s g-)(n e4 q)"
            "(n c3 q p2 v2)(n d3 e l+)(n d3 e)"
            ")) )))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 136L );

        soCursor.move_next();
        find_next_break(breaker, soCursor);

//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 150L );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_451)
    {
        //@ 451. multimetric score. breaks at common barlines

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData "
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

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);
        //cout << test_name() << endl;
        //cout << pScore->get_staffobjs_table()->dump() << endl;

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);
        //cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 147L );
        CHECK( is_equal_time(soCursor.time(), 384.0f) );

    }

//    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_501)
//    {
//        //@ 501. Single instrument, no TS, but has barlines.
//        //@      Break after barlines
//
//        Document doc(m_libraryScope);
//        doc.from_string("(score (vers 2.0) "
//            "(instrument#121L (musicData (clef G)"
//            "(n c4 q)(n d4 q)(n e4 q)(barline)"
//            "(n c4 q)(n d4 q)(n e4 q)(barline)"
//            "(n c4 q)(n d4 q)(n e4 q)(barline)"
//            ")))" );
//
//        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
//        StaffObjsCursor soCursor(pScore);
//
//        MyColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
//        CHECK( breaker.is_in_barlines_mode() );
//
//        find_next_break(breaker, soCursor);
//
//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
//        CHECK( soCursor.imo_object()->get_id() == 128L );
//
//        soCursor.move_next();
//        find_next_break(breaker, soCursor);
//
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
//        CHECK( soCursor.imo_object()->get_id() == 132L );
//    }

    TEST_FIXTURE(ColumnBreakerTestFixture, column_breaker_551)
    {
        //@ 551. multi-instrument, no TSs
        //@      Do not break just before a barline

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument#121L (musicData (clef G)(key C)"
            "(n c4 e g+)(n d4 e g-)(barline)(n e4 q) ))"
            "(instrument (musicData (clef F4)(key C)"
            "(n c3 q)(barline)(n d3 q) ))"
            ")))" );

        ImoScore* pScore = static_cast<ImoScore*>( doc.get_im_root()->get_content_item(0) );
        StaffObjsCursor soCursor(pScore);

        ColumnBreaker breaker(pScore->get_num_instruments(), &soCursor);
        find_next_break(breaker, soCursor);

//        cout << test_name() << endl;
//        cout << pScore->get_staffobjs_table()->dump() << endl;
//        cout << "cursor id=" << soCursor.imo_object()->get_id() << endl;
        CHECK( soCursor.imo_object()->get_id() == 133L );
        CHECK( is_equal_time(soCursor.time(), 64.0f) );
    }

}


