//----------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
#include "lomse_system_cursor.h"
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
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~ColumnBreakerTestFixture()    //TearDown fixture
    {
    }

    void find_next_break(ColumnBreaker& breaker, SystemCursor& sysCursor)
    {
        while(!sysCursor.is_end() )
        {
            ImoStaffObj* pSO = sysCursor.get_staffobj();
//            int iInstr = sysCursor.num_instrument();
//            int iStaff = sysCursor.staff();
            int iLine = sysCursor.line();
            float rTime = sysCursor.time();

            if ( breaker.column_should_be_finished(pSO, rTime, iLine) )
                break;

            sysCursor.move_next();
        }
    }

};

SUITE(ColumnBreakerTest)
{

    TEST_FIXTURE(ColumnBreakerTestFixture, OneInstrument_AfterBarline)
    {
        //@ in one staff scores, break after barlines

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(n c4 q)(barline)(n d4 q)(barline)"
            "(n e4 q) ))) ))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
//        pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 10 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, OneInstrument_AfterEndOfBeam)
    {
        //@ in one staff scores, break after end-of beam notes

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData "
            "(clef G)(key C)(n a4 s g+)(n g4 s)(n f4 s)(n e4 s g-)"
            "(n d4 q)(n b4 q)(n c4 q)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 13 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, OneInstrument_AfterNotBeamedNoterest)
    {
        //@ in one staff scores, break after end-of beam notes

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData "
            "(clef G)(key C)(n a4 s)(r s)(n f4 s)(n e4 s)"
            "(n d4 q)(n b4 q)(n c4 q)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 10 );
        CHECK( is_equal_time(sysCursor.time(), 16.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, CommonOnsetPoint)
    {
        //@ in one staff scores, break after end-of beam notes

        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2) (musicData "
            "(clef G p1)(clef F4 p2)(key A-)"
            "(n -a4 w v1 p1)(n a4 h v1)(n xa4 h v1)(n a4 q v1)"
            "(n a4 q v1)(n +a4 e g+ v1)(n a4 e g- v1)(n a4 s g+ v1)"
            "(n a4 s g- v1)(n a4 t g+ v1)(n a4 t g- v1)"
            "(goBack start)(n f3 w v2 p2)(n +f3 h v2)(n f3 h v2)"
            "(n -f3 q v2)(n f3 q v2)(n f3 e g+ v2)(n +f3 e g- v2)"
            "(n f3 s g+ v2)(n f3 s g- v2)(n f3 t g+ v2)(n f3 t g- v2)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
//        pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 12 );
        CHECK( is_equal_time(sysCursor.time(), 256.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, DoNotBreakVoice)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 h p1)(n d4 q p1)(goBack start)(n c3 q p2)(n d3 q p2)(n e3 q p2)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 12 );
        CHECK( is_equal_time(sysCursor.time(), 128.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, DoNotBreakBeam_1)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 e p1 g+)(n d4 s p1)(n e4 s p1 g-)(n f4 s p1 g+)(n g4 s p1 g-)"
            "(goBack start)(n c3 q p2)(n d3 q p2)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 14 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, DoNotBreakBeam_2)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (staves 2) (musicData (clef G p1)(clef F4 p2)(key C)"
            "(n c4 s p1 g+)(n d4 s p1)(n e4 s p1)(n f4 s p1 g-)(n g4 q p1)"
            "(goBack start)(n c3 e p2)(r e p2)(n e3 e p2)(n g3 e p2 g+)(n b3 e p2 g-)"
            ")) )))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 15 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, NotBeforeBarline)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key C)"
            "(n c4 e g+)(n d4 e g-)(barline)(n e4 q) ))"
            "(instrument (musicData (clef F4)(key C)"
            "(n c3 q)(barline)(n d3 q) ))"
            ")))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        CHECK( sysCursor.imo_object()->get_id() == 12 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, Multimetrics_DoNotBreakBeams)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key G)(time 3 4)"
            "(n g5 s g+)(n f5 s)(n g5 e g-)(barline)(n g5 q) ))"
            "(instrument (musicData (clef G)(key G)(time 2 4)"
            "(n b5 e g+)(n a5 s)(n g5 s g-)(n g5 e g+)(n g5 e g-) ))"
            ")))" );

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker, sysCursor);

        //cout << "break at id = " << sysCursor.imo_object()->get_id() << endl;
        CHECK( sysCursor.imo_object()->get_id() == 14 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );
    }

    TEST_FIXTURE(ColumnBreakerTestFixture, Multimetrics_Ok)
    {
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

        ImoScore* pScore = doc.get_score();
        SystemCursor sysCursor(pScore);
        //pScore->get_staffobjs_table()->dump();

        ColumnBreaker breaker1(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker1, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 13 );
        CHECK( is_equal_time(sysCursor.time(), 64.0f) );

        ColumnBreaker breaker2(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker2, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 15 );
        CHECK( is_equal_time(sysCursor.time(), 128.0f) );

        ColumnBreaker breaker3(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker3, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 19 );
        CHECK( is_equal_time(sysCursor.time(), 192.0f) );

        ColumnBreaker breaker4(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker4, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 52 );
        CHECK( is_equal_time(sysCursor.time(), 256.0f) );

        ColumnBreaker breaker5(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker5, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 55 );
        CHECK( is_equal_time(sysCursor.time(), 320.0f) );

        ColumnBreaker breaker6(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker6, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 22 );
        CHECK( is_equal_time(sysCursor.time(), 384.0f) );

        ColumnBreaker breaker7(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker7, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 25 );
        CHECK( is_equal_time(sysCursor.time(), 512.0f) );

        ColumnBreaker breaker8(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker8, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 29 );
        CHECK( is_equal_time(sysCursor.time(), 576.0f) );

        ColumnBreaker breaker9(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker9, sysCursor);
        CHECK( sysCursor.imo_object()->get_id() == 31 );
        CHECK( is_equal_time(sysCursor.time(), 640.0f) );

        ColumnBreaker breaker10(pScore->get_num_instruments(), &sysCursor);
        find_next_break(breaker10, sysCursor);
        //cout << "break at id = " << sysCursor.imo_object()->get_id() << endl;
        CHECK( sysCursor.imo_object()->get_id() == 33 );
        CHECK( is_equal_time(sysCursor.time(), 704.0f) );
    }

//    TEST_FIXTURE(ColumnBreakerTestFixture, DumpStaffobsTable)
//    {
//        Document doc(m_libraryScope);
//        doc.from_file(m_scores_path + "00201-systems-are-justified.lms");
//        ImoScore* pScore = doc.get_score();
//        pScore->get_staffobjs_table()->dump();
//    }

}


