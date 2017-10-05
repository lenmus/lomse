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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_document_cursor.h"
#include "lomse_compiler.h"
#include "lomse_time.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// DocContentCursor tests
//=======================================================================================

//---------------------------------------------------------------------------------------
//derived class to have access to some protected methods
class MyDocContentCursor : public DocContentCursor
{
public:
    MyDocContentCursor(Document* pDoc) : DocContentCursor(pDoc) {}

};

//---------------------------------------------------------------------------------------
class DocContentCursorTestFixture
{
public:

    DocContentCursorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
    }

    ~DocContentCursorTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }

    void create_document_1()
    {
        //"(lenmusdoc (vers 0.0) (content "
        //    "(score#80L (vers 2.0) "
        //        "(instrument#101L (musicData#102L (clef#103L G)(key#104L C)"
        //        "(time#105L 2 4)(n#106L c4 q)(r#107L q) )))"
        //    "(para#108L (txt#109L \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#80L (vers 2.0) "
                "(instrument#101L (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
        //cout << m_pDoc->to_string(true) << endl;
    }

    void create_empty_document()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))");
    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
};

SUITE(DocContentCursorTest)
{
    TEST_FIXTURE(DocContentCursorTestFixture, points_to_start_of_content)
    {
        //001. initialy points to first element in content
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, empty_doc_points_to_end_of_content)
    {
        //002. if document is empty points to 'end-of-document' value (NULL)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        MyDocContentCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, next)
    {
        //003. next: advances to end of document
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, next_remains_at_end)
    {
        //004. next: when at end of document remains there
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev)
    {
        //005. prev: moves back to previous
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 80L );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_before_first)
    {
        //006. prev: when at p_start_cursor() remains there
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_from_end)
    {
        //007 prev: when at end, moves to last element
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 80L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_from_end_in_empty_doc)
    {
        //008 prev: when at end in empty doc, remains at end
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        MyDocContentCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );

        --cursor;

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id)
    {
        //009. point to, by id: goes to element if exists and is top level
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        //cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(110L);
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 80L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_found)
    {
        //010. point to, by id: if not found go to end
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(175L);
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_top_level)
    {
        //011 point to, by id: if not top level go to end
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(106L);       //time signature
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_ptr_1)
    {
        //012 point to, by ptr: goes to element if top level
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(110L);   //move to paragraph to get pointer to it
        ImoBlockLevelObj* pImo = static_cast<ImoBlockLevelObj*>( *cursor );
        cursor.point_to(174L);  //to end
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );

        cursor.point_to(pImo);

        CHECK( *cursor != NULL );
        CHECK( *cursor == pImo );
        CHECK( cursor.get_pointee_id() == 110L );
        CHECK( cursor.get_prev_id() == 80L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_ptr_2)
    {
        //013 point to, by ptr: if not top level, go to end
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ImoScore* pScore =  static_cast<ImoScore*>( *cursor );
        ImoObj* pClef = pScore->get_instrument(0)->get_musicdata()->get_child(0);
        CHECK( pClef->is_clef() == true );

        cursor.point_to(pClef);  //to end

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 110L );
    }

//    TEST_FIXTURE(DocContentCursorTestFixture, prev_when_added_object)
//    {
//        //51. move prev ok when adding element
//        create_document_1();
//        MyDocContentCursor cursor(m_pDoc);
//        cursor.move_next();
//        CHECK( cursor.get_pointee_id() == 26L );
//        CHECK( cursor.get_prev_id() == 80L );
//        ImoDocument* pImoDoc = m_pDoc->get_imodoc();
//
//        ImoParagraph* pImo = pImoDoc->add_paragraph();
//        ImoId id = pImo->get_id();
//        cout << "id=" << id << endl;
//
//        //cursor is not aware of changes
//        CHECK( *cursor != NULL );
//        CHECK( cursor.get_pointee_id() == 26L );
//        CHECK( cursor.get_prev_id() == 80L );
//
//        //but moves backwards ok
//        --cursor;
//
//        CHECK( cursor.get_pointee_id() == 80L );
//        CHECK( cursor.get_prev_id() == k_cursor_before_start );
//
//        //and traverses ok the inserted element
//        ++cursor;
//
//        CHECK( (*cursor)->is_paragraph() == true );
//        CHECK( cursor.get_pointee_id() == id );
//        CHECK( cursor.get_prev_id() == 80L );
//        cout << "id=" << cursor.get_pointee_id() << endl;
//
//        ++cursor;
//
//        CHECK( cursor.get_pointee_id() == 26L );
//        CHECK( cursor.get_prev_id() == id );
//        cout << "id=" << cursor.get_prev_id() << endl;
//    }

//    TEST_FIXTURE(DocContentCursorTestFixture, update_after_deletion)
//    {
//        //@ moves to next object when current has been deleted
//        create_document_1();
//        MyDocContentCursor cursor(m_pDoc);
//        cursor.point_to(4L);  //score
////        CHECK( (*cursor)->is_paragraph() == true );
//        CHECK (false);
//    }
//
//    TEST_FIXTURE(DocContentCursorTestFixture, update_after_deletion_end)
//    {
////        //@ moves to end when current has been deleted and is last one
////        create_empty_document();
////        MyDocContentCursor cursor(m_pDoc);
////        CHECK( *cursor == NULL );
////        ImoDocument* pImoDoc = m_pDoc->get_imodoc();
////        pImoDoc->add_paragraph();
////        CHECK( *cursor == NULL );
////        --cursor;
////        CHECK( (*cursor)->is_paragraph() == true );
//        CHECK (false);
//    }

};



//=======================================================================================
// DocCursor tests
//=======================================================================================

//derivate class to have access to some protected methods
class MyDocCursor : public DocCursor
{
public:
    MyDocCursor(Document* pDoc) : DocCursor(pDoc) {}

    //access to protected members
    bool my_is_jailed() { return is_jailed(); }

    //debug
    void dump_ids()
    {
        cout << "pointee_id=" << get_pointee_id()
             << ", parent_id=" << get_parent_id()
             << endl;
    }

};


//---------------------------------------------------------------------------------------
class DocCursorTestFixture
{
public:

    DocCursorTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
        , m_scores_path(TESTLIB_SCORES_PATH)
        , m_pDoc(NULL)
    {
    }

    ~DocCursorTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = NULL;
    }

    void create_document_1()
    {
        //"(lenmusdoc (vers 0.0) (content "
        //    "(score#80L (vers 2.0) "
        //        "(instrument#101L (musicData#102L (clef#103L G)(key#104L C)"
        //        "(time#105L 2 4)(n#106L c4 q)(r#107L q) )))"
        //    "(para#108L (txt#109L \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#80L (vers 2.0) "
                "(instrument#101L (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
        //cout << m_pDoc->to_string(true) << endl;
    }

    void create_document_3()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "09007-score-in-exercise.lmd",
                          Document::k_format_lmd );
    }

    void create_document_4()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_file(m_scores_path + "09008-score-in-exercise.lmd",
                          Document::k_format_lmd );
    }

    void create_empty_document()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))");
    }

    LibraryScope m_libraryScope;
    std::string m_scores_path;
    Document* m_pDoc;
};

SUITE(DocCursorTest)
{
    TEST_FIXTURE(DocCursorTestFixture, creation_101)
    {
        //101. when created points to first element in content
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, creation_102)
    {
        //102. if document is empty points to 'end-of-document' value (NULL)
        create_empty_document();
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, creation_103)
    {
        //103. copy constructor
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.enter_element();     //clef 103L
        ++cursor;                   //key 104L
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );

        DocCursor cursor2(&cursor);
        //cout << (*cursor2)->to_string() << endl;

        CHECK( *cursor2 != NULL );
        CHECK( (*cursor2)->is_key_signature() == true );
        CHECK( cursor2.get_pointee_id() == 104L );
        CHECK( cursor2.get_parent_id() == 80L );
        CHECK( cursor2.is_inside_terminal_node() == true );

        ++cursor2;
        CHECK( (*cursor2)->to_string() == "(time 2 4)" );
        CHECK( (*cursor)->to_string() == "(key C)" );
    }

    TEST_FIXTURE(DocCursorTestFixture, creation_104)
    {
        //104. copy constructor. Several layers
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(68L);       //key in score
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 68L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );

        DocCursor cursor2(&cursor);
        //cout << (*cursor2)->to_string() << endl;

        CHECK( *cursor2 != NULL );
        CHECK( (*cursor2)->is_key_signature() == true );
        CHECK( cursor2.get_pointee_id() == 68L );
        CHECK( cursor2.get_parent_id() == 64L );
        CHECK( cursor2.is_inside_terminal_node() == true );

        ++cursor2;      //time
        CHECK( cursor2.get_pointee_id() == 69L );
        CHECK( cursor2.get_parent_id() == 64L );
        CHECK( cursor.get_pointee_id() == 68L );
        CHECK( cursor.get_parent_id() == 64L );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_next_201)
    {
        //201. move forwards until end-of-document and remains there
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_next_202)
    {
        //202. move forwards until end-of-document and remains there. Several layers
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 14L );
        CHECK( cursor.get_parent_id() == 14L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 16L );
        CHECK( cursor.get_parent_id() == 16L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 19L );
        CHECK( cursor.get_parent_id() == 18L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 18L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 23L );
        CHECK( cursor.get_parent_id() == 22L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 104L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        //cursor.dump_ids();

        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_301)
    {
        //301. move backwards until first object and remains there
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
        --cursor;
        CHECK( (*cursor)->is_score() == true );

        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_302)
    {
        //302. move backwards until first object and remains there. Several levels
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(104L);
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 104L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 23L );
        CHECK( cursor.get_parent_id() == 22L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 18L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 19L );
        CHECK( cursor.get_parent_id() == 18L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 16L );
        CHECK( cursor.get_parent_id() == 16L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 14L );
        CHECK( cursor.get_parent_id() == 14L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 14L );
        CHECK( cursor.get_parent_id() == 14L );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_303)
    {
        //303 to last element if pointing to 'end-of-document' value
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 110L );
        CHECK( cursor.get_parent_id() == 110L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_304)
    {
        //304 to last element if pointing to 'end-of-document' value. Several levels
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(64L);   //last score
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_305)
    {
        //305 to added element when doc was empty
        create_empty_document();
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );

        ImoDocument* pImoDoc = m_pDoc->get_imodoc();
        pImoDoc->add_paragraph();

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_parent_id() == k_cursor_at_end );
        CHECK( cursor.is_inside_terminal_node() == false );

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, enter_element_401)
    {
        //401. enter: if pointing to a top level element, delegates
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        CHECK( cursor.get_pointee_id() == cursor.get_parent_id() );
        CHECK( cursor.is_inside_terminal_node() == false );

        cursor.enter_element();

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 103L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, enter_element_402)
    {
        //402. enter: attempt to enter non-top element does nothing
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.enter_element();     //enter score: points to clef

        cursor.enter_element();     //trying to enter into clef

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 103L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_501)
    {
        //501. point to, by id. Positioning at content level element
        create_document_1();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(110L);   //paragraph

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 110L );
        CHECK( cursor.get_parent_id() == 110L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_502)
    {
        //502. point to, by id. Removes delegation
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.enter_element();         //enter score
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( (*cursor)->is_clef() == true );

        cursor.point_to(110L);   //paragraph

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 110L );
        CHECK( cursor.get_parent_id() == 110L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_503)
    {
        //503. point to, by id. If not found goes to end
        create_document_1();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(175L);

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_parent_id() == k_cursor_at_end );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_504)
    {
        //504. point to, by id. Positioning at inner element
        create_document_1();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(106L);   //first note

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == 106L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_505)
    {
        //505. point to, by id. moving to a sibbling
        create_document_1();
        MyDocCursor cursor(m_pDoc);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(106L);   //first note
        CHECK( (*cursor)->is_note() == true )
        ;
        cursor.point_to(104L);   //key signature

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_506)
    {
        //506. point to, by id. Positioning at non-terminal at inner level
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(64L);   //score

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_507)
    {
        //507. point to, by id. Positioning at terminal at inner level
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(71L);   //first note in score

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == 71L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, to_start_601)
    {
        //601. to_start. Not delegating moves to first top level
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(110L);   //paragraph

        cursor.to_start();

        CHECK( cursor.get_pointee_id() == 80L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, to_start_602)
    {
        //602. to_start. Delegating, stops delegation and moves to first top level
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //note

        cursor.to_start();

        CHECK( cursor.get_pointee_id() == 80L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_701)
    {
        //701. get state. Non-terminal at first level, pointing object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;       //paragraph

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 110L );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_702)
    {
        //702. get state. Non-terminal at first level at end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == k_cursor_at_end );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_703)
    {
        //703. get state. Parent is first level. delegating.
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //first note

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 80L );     //score
        CHECK( state.get_delegate_state() != NULL );
        ScoreCursorState* pSCE = dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE->id() == 106L );
        CHECK( pSCE->instrument() == 0 );
        CHECK( pSCE->measure() == 0 );
        CHECK( pSCE->staff() == 0 );
        CHECK( is_equal_time(pSCE->time(), 0.0f) );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_704)
    {
        //704. get state. delegating. Parent is first level. At end of score
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(107L);   //last rest
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 80L );     //score
        CHECK( state.get_delegate_state() != NULL );
        ScoreCursorState* pSCE =
            dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE->is_at_end_of_score() );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_705)
    {
        //705. get state. Non-terminal at inner level, pointing object
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);       //paragraph inside content

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 20L );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_706)
    {
        //706. get state. Non-terminal at inner level at end
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(64L);       //score 64L inside content 14L
        ++cursor;                   //at end
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == k_cursor_at_end );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_707)
    {
        //707. get state. Parent is inner level. delegating
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(71L);   //first note in score 64L

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 64L );     //score
        CHECK( state.get_delegate_state() != NULL );
        ScoreCursorState* pSCE = dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE->id() == 71L );
        CHECK( pSCE->instrument() == 0 );
        CHECK( pSCE->measure() == 0 );
        CHECK( pSCE->staff() == 0 );
        CHECK( is_equal_time(pSCE->time(), 0.0f) );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_708)
    {
        //708. get state. Parent is inner level. delegating. At end of score
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(101L);   //last barline in score 64L
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        DocCursorState state = cursor.get_state();
        //cout << cursor.dump_cursor() << endl;
        //cursor.dump_ids();

        CHECK( state.get_parent_level_id() == 64L );     //score
        CHECK( state.get_delegate_state() != NULL );
        ScoreCursorState* pSCE =
            dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE->is_at_end_of_score() );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_720)
    {
        //720. restore_state. Non-terminal at first level, pointing object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;       //paragraph 108L
        DocCursorState state = cursor.get_state();
        CHECK( state.get_parent_level_id() == 110L );
        CHECK( state.get_delegate_state() == NULL );

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 110L );
        CHECK( cursor.get_parent_id() == 110L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_721)
    {
        //721. restore_state. Terminal, pointing object. Parent at first level.
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //first note
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 106L );
        CHECK( cursor.get_parent_id() == 80L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_722)
    {
        //722. restore_state. Non-terminal at inner level, pointing object
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);       //paragraph 12L inside content 10L
        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 18L );
        DocCursorState state = cursor.get_state();
        CHECK( state.get_parent_level_id() == 20L );
        CHECK( state.get_delegate_state() == NULL );

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 18L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cout << cursor.dump_cursor() << endl;
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_723)
    {
        //723. restore_state. Terminal, pointing object. Parent at inner level.
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(71L);   //first note, score 64
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 71L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_724)
    {
        //724. restore state. Non-terminal at first level at end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( state.get_parent_level_id() == k_cursor_at_end );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_725)
    {
        //725. restore state. Parent is first level. delegating. At end of score
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(107L);   //last rest
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 80L );
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 80L );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_726)
    {
        //726. restore state. Non-terminal at inner level at end
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(64L);       //score 64 inside content 14
        ++cursor;                   //at end
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( state.get_parent_level_id() == k_cursor_at_end );
        CHECK( state.get_delegate_state() == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_727)
    {
        //727. restore state. Parent is inner level. delegating. At end of score
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(101L);   //last barline in score 64
        ++cursor;
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 64L );
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 64L );
    }

    TEST_FIXTURE(DocCursorTestFixture, delegating_801)
    {
        //801. move next: top level not modified and delegate move
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //first note

        cursor.move_next();      //to rest #107L

        CHECK( cursor.get_pointee_id() == 107L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, delegating_802)
    {
        //802. move prev: top level not modified and delegate move
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(107L);    //last rest

        cursor.move_prev();      //to note #106L

        CHECK( cursor.get_pointee_id() == 106L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, surviving_updates_810)
    {
        //810. when first object deleted, go to new first object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoScore* pScore = static_cast<ImoScore*>( *cursor );
        ImoDocument* pImoDoc = pScore->get_document();
        pImoDoc->erase(pScore);

        cursor.reset_and_point_after(id);

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, surviving_updates_811)
    {
        //811. when last object is deleted, go to end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;   //paragraph
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoParagraph* pImo = static_cast<ImoParagraph*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);

        cursor.reset_and_point_after(id);     //should move to end

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, surviving_updates_812)
    {
        //812. when any other object is deleted, go to next object
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 2.0) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para (txt \"Hello world!\"))"
            "(heading 1 (txt \"The title!\"))"
            "))" );
        MyDocCursor cursor(m_pDoc);
        ++cursor;   //points to paragraph
        CHECK( (*cursor)->is_paragraph() );
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoParagraph* pImo = static_cast<ImoParagraph*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);

        cursor.reset_and_point_after(id);

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_heading() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, surviving_updates_813)
    {
        //813. when last object is deleted, go to end. Deleted object at inner level
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(64L);   //score
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoScore* pImo = static_cast<ImoScore*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);

        cursor.reset_and_point_after(id);     //should move to end

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor == NULL );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        //cout << cursor.dump_cursor() << endl;
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, surviving_updates_814)
    {
        //814. when any other object is deleted, go to next object. At inner level.
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);   //paragraph
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoParagraph* pImo = static_cast<ImoParagraph*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);

        cursor.reset_and_point_after(id);     //should move to paragraph 15

        CHECK( (*cursor)->is_paragraph() );
        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( cursor.get_pointee_id() == 23L );
        CHECK( cursor.get_parent_id() == 22L );

        ++cursor;
        CHECK( (*cursor)->is_score() );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_820)
    {
        //820. when jailed mode, enters element and moves to first child
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        bool fJailed = cursor.jailed_mode_in(64L);    //score

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 67L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        CHECK( fJailed == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_821)
    {
        //821. cannot be jailed in non-terminal element
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        bool fJailed = cursor.jailed_mode_in(71L);    //first note

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 14L );
        CHECK( cursor.get_parent_id() == 14L );
        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( cursor.my_is_jailed() == false );
        CHECK( fJailed == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_822)
    {
        //822. cannot exit element
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.jailed_mode_in(64L);    //score

        cursor.exit_element();

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 67L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_823)
    {
        //823. exiting jailed mode allows moving out
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.jailed_mode_in(64L);    //score

        cursor.terminate_jailed_mode();
        cursor.exit_element();

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 22L );
        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( cursor.my_is_jailed() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_824)
    {
        //824. cannot point_to an element out of parent box
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.jailed_mode_in(64L);    //score

        cursor.point_to(20L);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 67L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_825)
    {
        //825. but can point_to elements inside parent box
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.jailed_mode_in(64L);    //score

        cursor.point_to(101L);  //last barline

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( cursor.get_pointee_id() == 101L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_826)
    {
        //826. cannot restore state to point to an element out of parent box
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);       //paragraph inside content
        DocCursorState state = cursor.get_state();
        cursor.jailed_mode_in(64L);    //score, pointing to clef 67L

        cursor.restore_state(state);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 67L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_827)
    {
        //827. but can restore state to point to elements inside parent box
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(101L);       //score, last barline
        DocCursorState state = cursor.get_state();
        cursor.jailed_mode_in(64L);    //score, pointing to clef 67

        cursor.restore_state(state);

        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( cursor.get_pointee_id() == 101L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

}

