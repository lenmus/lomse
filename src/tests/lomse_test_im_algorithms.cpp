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
#include "lomse_im_algorithms.h"
#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_staffobjs_table.h"
#include "lomse_document_cursor.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


////---------------------------------------------------------------------------------------
//// helper, for accessing protected members
//class MyImoTreeAlgoritms : public ImoTreeAlgoritms
//{
//public:
//    MyImoTreeAlgoritms(ImoScore* pScore) : ImoTreeAlgoritms(pScore) {}
//    ~MyImoTreeAlgoritms() {}
//
//    //access to protected members
//    ColStaffObjs* my_get_colstaffobjs() { return m_pColStaffObjs; }
//    bool my_is_necessary_to_shift_time_back_from(ImoStaffObj* pSO) {
//            return is_necessary_to_shift_time_back_from(pSO);
//    }
//    void my_shift_objects_back_in_time_from(ImoStaffObj* pSO) {
//            shift_objects_back_in_time_from(pSO);
//    }
//};

//---------------------------------------------------------------------------------------
class ImoTreeAlgoritmsTestFixture
{
public:

    ImoTreeAlgoritmsTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
    }

    ~ImoTreeAlgoritmsTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
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
            "(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)"
                "(r q)(barline)(n d4 q)"
                ")))"
            "))" );
        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
    }

//TODO: REVIEW SCORES, AS THEY HAVE BEEN CHANGED TO VERS. 2.0
//    void create_document_2()
//    {
//        //An score with empty places in first staff (after 26L, an in second measure)
////       (score#15 (vers 2.0)
////       (instrument#19 (staves 2) (musicData#20
////       (clef#21 G p1)(clef#22 F4 p2)(key#23 C)(time#24 2 4)
////       (n#25 e4 e g+ p1)(n#26 g4 e g-)(goBack#32 start)
////       (n#33 c3 e g+ p2)(n#34 e3 e g-)(n#40 g3 e g+)(n#41 c4 e g-)
////       (barline#47)
////       (n#48 a3 q p2)(n#49 e3 q)
////       (barline#50) )))
//
//        m_pDoc = LOMSE_NEW Document(m_libraryScope);
//        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
//            "(score (vers 2.0)"
//               "(instrument (staves 2) (musicData "
//               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
//               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
//               "(n c3 e g+ p2)(n e3 e g-)(n g3 e g+)(n c4 e g-)"
//               "(barline)"
//               "(n a3 q p2)(n e3 q)"
//               "(barline) )))"
//            "))" );
//        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//    }
//
//    void create_document_3()
//    {
//        //As score 2 but without last barline
//        m_pDoc = LOMSE_NEW Document(m_libraryScope);
//        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
//            "(score (vers 2.0)"
//               "(instrument (staves 2) (musicData "
//               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
//               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
//               "(n c3 e g+ p2)(n e3 e g-)(n g3 e g+)(n c4 e g-)"
//               "(barline)"
//               "(n a3 q p2)(n e3 q) )))"
//            "))" );
//        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
//    }
//
//    void create_document_4()
//    {
//        //As score with emptypositions defined by time grid
//        // (score#15 (vers 2.0)
//        //    (instrument#19 (staves 2) (musicData#20
//        //      (clef#21 G p1)(clef#22 F4 p2)(key#23 C)(time#24 2 4)
//        //      (n#25 e4 e g+ p1)(n#26 g4 e g-)(goBack start)
//        //      (n#33 c3 e g+ p2)(n#34 e3 e g-)(n#40 g3 q)
//        //      (barline#41) )))
//        m_pDoc = LOMSE_NEW Document(m_libraryScope);
//        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
//            "(score (vers 2.0)"
//               "(instrument (staves 2) (musicData "
//               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
//               "(n e4 e g+ p1)(n g4 e g-)(goBack start)"
//               "(n c3 e g+ p2)(n e3 e g-)(n g3 q)"
//               "(barline) )))"
//            "))" );
//        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
////        cout << m_pDoc->to_string(k_save_ids) << endl;
//    }
//
//    void create_document_5()
//    {
//        //Score with voices
//        //(score#15 (vers 2.0)
//        //   (instrument#19 (staves 2) (musicData#20
//        //   (clef#21 G p1)(clef#22 F4 p2)(key#23 C)(time#24 2 4)
//        //   (n#25 c5 e p1 (stem up))(n#26 e5 e (stem up))(n#27 c5 e (stem up))(n#28 e5 e (stem up))(goBack start)
//        //   (n#30 e4 e p1 (stem down))(n#31 g4 e (stem down))(n#32 e4 e (stem down))(n#33 g4 e (stem down))(goBack start)
//        //   (n#35 c3 e p2 (stem up))(n#36 e3 e (stem up))(n#37 g3 q (stem up))(goBack start)
//        //   (n#39 g2 e p2 (stem down))(n#40 c3 e (stem down))(n#41 c3 e (stem down))(n#42 c3 e (stem down))(barline#43)
//        //   (n#44 c5 e p1 (stem up))(n#45 e5 e (stem up))(n#46 c5 e (stem up))(n#47 e5 e (stem up))(goBack start)
//        //   (n#49 e4 e p1 (stem down))(n#50 g4 e (stem down))(n#51 e4 e (stem down))(n#52 g4 e (stem down))(goBack start)
//        //   (n#54 c3 e p2 (stem up))(n#55 e3 e (stem up))(n#56 g3 q (stem up))(goBack start)
//        //   (n#58 g2 e p2 (stem down))(n#59 c3 e (stem down))(n#60 c3 e (stem down))(n#61 c3 e (stem down))(barline#62)
//
//        m_pDoc = LOMSE_NEW Document(m_libraryScope);
//        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
//            "(score (vers 2.0)"
//               "(instrument (staves 2) (musicData "
//               "(clef G p1)(clef F4 p2)(key C)(time 2 4)"
//               "(n c5 e p1 v1 (stem up))(n e5 e (stem up))(n c5 e (stem up))(n e5 e (stem up))(goBack start)"
//               "(n e4 e p1 v2 (stem down))(n g4 e (stem down))(n e4 e (stem down))(n g4 e (stem down))(goBack start)"
//               "(n c3 e p2 v3 (stem up))(n e3 e (stem up))(n g3 q (stem up))(goBack start)"
//               "(n g2 e p2 v4 (stem down))(n c3 e (stem down))(n c3 e (stem down))(n c3 e (stem down))(barline)"
//               "(n c5 e p1 v1 (stem up))(n e5 e (stem up))(n c5 e (stem up))(n e5 e (stem up))(goBack start)"
//               "(n e4 e p1 v2 (stem down))(n g4 e (stem down))(n e4 e (stem down))(n g4 e (stem down))(goBack start)"
//               "(n c3 e p2 v3 (stem up))(n e3 e (stem up))(n g3 q (stem up))(goBack start)"
//               "(n g2 e p2 v4 (stem down))(n c3 e (stem down))(n c3 e (stem down))(n c3 e (stem down))(barline)"
//               ")))"
//            "))" );
//        m_pScore = static_cast<ImoScore*>( m_pDoc->get_imodoc()->get_content_item(0) );
////        cout << m_pDoc->to_string(k_save_ids) << endl;
//    }

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

SUITE(ImoTreeAlgoritmsTest)
{

//    TEST_FIXTURE(ImoTreeAlgoritmsTestFixture, builds)
//    {
//        create_document_1();
//        MyImoTreeAlgoritms sm(m_pScore);
//        CHECK( sm.my_get_colstaffobjs() != NULL );
//    }
//
//    TEST_FIXTURE(ImoTreeAlgoritmsTestFixture, shift_back_1)
//    {
//        //delete note implies time shift
//        create_document_1();
//        MyImoTreeAlgoritms sm(m_pScore);
//        ScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(24L);   //first note
//        ImoStaffObj* pSO = static_cast<ImoStaffObj*>( *cursor );
//
//        CHECK( sm.my_is_necessary_to_shift_time_back_from(pSO) == true );
//    }
//
//    TEST_FIXTURE(ImoTreeAlgoritmsTestFixture, shift_back_2)
//    {
//        //delete clef does not implie time shift
//        create_document_1();
//        MyImoTreeAlgoritms sm(m_pScore);
//        ScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(21L);   //clef
//        ImoStaffObj* pSO = static_cast<ImoStaffObj*>( *cursor );
//
//        CHECK( sm.my_is_necessary_to_shift_time_back_from(pSO) == false );
//    }
//
//    TEST_FIXTURE(ImoTreeAlgoritmsTestFixture, shift_back_3)
//    {
//        //shift time
//        create_document_1();
//        MyImoTreeAlgoritms sm(m_pScore);
//        ScoreCursor cursor(m_pDoc, m_pScore);
//        cursor.point_to(24L);   //first note
//        ImoStaffObj* pSO = static_cast<ImoStaffObj*>( *cursor );
//
//        sm.my_shift_objects_back_in_time_from(pSO);
//
//        cursor.point_to(24L);
//
//        ++cursor;
//        CHECK( cursor.get_pointee_id() == 25L );        //rest
//        CHECK( is_equal_time(cursor.time(), 0.0f) );
//
//        ++cursor;
//        CHECK( cursor.get_pointee_id() == 26L );        //barline
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//
//        ++cursor;
//        CHECK( cursor.get_pointee_id() == 27L );        //first note in measure 2
//        CHECK( is_equal_time(cursor.time(), 64.0f) );
//    }
//
////    TEST_FIXTURE(ImoTreeAlgoritmsTestFixture, shift_back_4)
////    {
////        //shift time skips other instruments
////        create_document_5();
////        MyImoTreeAlgoritms sm(m_pScore);
////        ScoreCursor cursor(m_pDoc, m_pScore);
////        cursor.point_to(30L);   //first note contralto
////        ImoStaffObj* pSO = static_cast<ImoStaffObj*>( *cursor );
////
////        sm.my_shift_objects_back_in_time_from(pSO);
//////   n#25 n#26 n#27 n#28            n#44 n#45 n#46 n#47
//////   n#30 n#31 n#32 n#33            n#49 n#50 n#51 n#52
//////   n#35 n#36 n#37                 n#54 n#55 n#56
//////   n#39 n#40 n#41 n#42 barline#43 n#58 n#59 n#60 n#61 barline#62
////
//////        cout << m_pScore->get_staffobjs_table()->dump();
////
//////        ++cursor;
//////        CHECK( cursor.get_pointee_id() == 25L );
//////        CHECK( is_equal_time(cursor.time(), 0.0f) );
//////
//////        ++cursor;
//////        CHECK( cursor.get_pointee_id() == 26L );
//////        CHECK( is_equal_time(cursor.time(), 64.0f) );
//////
//////        ++cursor;
//////        CHECK( cursor.get_pointee_id() == 27L );
//////        CHECK( is_equal_time(cursor.time(), 64.0f) );
////    }

}

