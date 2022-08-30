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
#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
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
        , m_pDoc(nullptr)
    {
    }

    ~DocContentCursorTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = nullptr;
    }

    void create_document_1()
    {
        //"(lenmusdoc (vers 0.0) (content "
        //    "(score#50L (vers 2.0) "
        //        "(instrument (musicData#102L (clef#103L G)(key#104L C)"
        //        "(time#105L 2 4)(n#106L c4 q)(r#107L q) )))"
        //    "(para#120L (txt#121L \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#50L (vers 2.0) "
                "(instrument#90L (musicData#102L (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para#120L (txt \"Hello world!\"))"
            "))" );
//        cout << m_pDoc->dump_tree() << endl;
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
        //002. if document is empty points to 'end-of-document' value (nullptr)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        MyDocContentCursor cursor(m_pDoc);
        CHECK( *cursor == nullptr );
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
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, next_remains_at_end)
    {
        //004. next: when at end of document remains there
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev)
    {
        //005. prev: moves back to previous
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 50L );
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
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 50L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_from_end_in_empty_doc)
    {
        //008 prev: when at end in empty doc, remains at end
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        MyDocContentCursor cursor(m_pDoc);
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );

        --cursor;

        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == k_cursor_before_start );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id)
    {
        //009. point to, by id: goes to element if exists and is top level
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        //cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(120L);
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_prev_id() == 50L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_found)
    {
        //010. point to, by id: if not found go to end
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(175L);
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_top_level)
    {
        //011 point to, by id: if not top level go to end
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(106L);       //time signature
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_ptr_1)
    {
        //012 point to, by ptr: goes to element if top level
        create_document_1();
        MyDocContentCursor cursor(m_pDoc);
        cursor.point_to(120L);   //move to paragraph to get pointer to it
        ImoBlockLevelObj* pImo = static_cast<ImoBlockLevelObj*>( *cursor );
        cursor.point_to(174L);  //to end
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );

        cursor.point_to(pImo);

        CHECK( *cursor != nullptr );
        CHECK( *cursor == pImo );
        CHECK( cursor.get_pointee_id() == 120L );
        CHECK( cursor.get_prev_id() == 50L );
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

        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        CHECK( cursor.get_prev_id() == 120L );
    }

//    TEST_FIXTURE(DocContentCursorTestFixture, prev_when_added_object)
//    {
//        //51. move prev ok when adding element
//        create_document_1();
//        MyDocContentCursor cursor(m_pDoc);
//        cursor.move_next();
//        CHECK( cursor.get_pointee_id() == 26L );
//        CHECK( cursor.get_prev_id() == 50L );
//        ImoDocument* pImoDoc = m_pDoc->get_im_root();
//
//        ImoParagraph* pImo = pImoDoc->add_paragraph();
//        ImoId id = pImo->get_id();
//        cout << "id=" << id << endl;
//
//        //cursor is not aware of changes
//        CHECK( *cursor != nullptr );
//        CHECK( cursor.get_pointee_id() == 26L );
//        CHECK( cursor.get_prev_id() == 50L );
//
//        //but moves backwards ok
//        --cursor;
//
//        CHECK( cursor.get_pointee_id() == 50L );
//        CHECK( cursor.get_prev_id() == k_cursor_before_start );
//
//        //and traverses ok the inserted element
//        ++cursor;
//
//        CHECK( (*cursor)->is_paragraph() == true );
//        CHECK( cursor.get_pointee_id() == id );
//        CHECK( cursor.get_prev_id() == 50L );
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
////        CHECK( *cursor == nullptr );
////        ImoDocument* pImoDoc = m_pDoc->get_im_root();
////        pImoDoc->add_paragraph();
////        CHECK( *cursor == nullptr );
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
        , m_pDoc(nullptr)
    {
    }

    ~DocCursorTestFixture()    //TearDown fixture
    {
        delete m_pDoc;
        m_pDoc = nullptr;
    }

    void create_document_1()
    {
        //"(lenmusdoc (vers 0.0) (content "
        //    "(score#50L (vers 2.0) "
        //        "(instrument#90L (musicData#102L (clef#103L G)(key#104L C)"
        //        "(time#105L 2 4)(n#106L c4 q)(r#107L q) )))"
        //    "(para#120L (txt#121L \"Hello world!\"))"
        //"))"
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score#50L (vers 2.0) "
                "(instrument#90L (musicData#102L (clef G)(key C)(time 2 4)(n c4 q)(r q) )))"
            "(para#120L (txt \"Hello world!\"))"
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
        //102. if document is empty points to 'end-of-document' value (nullptr)
        create_empty_document();
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == nullptr );
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
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == true );

        DocCursor cursor2(&cursor);
        //cout << (*cursor2)->to_string() << endl;

        CHECK( *cursor2 != nullptr );
        CHECK( (*cursor2)->is_key_signature() == true );
        CHECK( cursor2.get_pointee_id() == 104L );
        CHECK( cursor2.get_parent_id() == 50L );
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
        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 68L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );

        DocCursor cursor2(&cursor);
        //cout << (*cursor2)->to_string() << endl;

        CHECK( *cursor2 != nullptr );
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
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        ++cursor;
        CHECK( *cursor == nullptr );
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
        CHECK( cursor.get_pointee_id() == 15L );
        CHECK( cursor.get_parent_id() == 15L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 17L );
        CHECK( cursor.get_parent_id() == 17L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 20 );
        CHECK( cursor.get_parent_id() == 19L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 21L );
        CHECK( cursor.get_parent_id() == 19L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 24L );
        CHECK( cursor.get_parent_id() == 23L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 105L );
        CHECK( cursor.get_parent_id() == 105L );
        //cursor.dump_ids();

        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );
        //cursor.dump_ids();

        ++cursor;
        CHECK( *cursor == nullptr );
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
        cursor.point_to(105L);
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 105L );
        CHECK( cursor.get_parent_id() == 105L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 24L );
        CHECK( cursor.get_parent_id() == 23L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 21L );
        CHECK( cursor.get_parent_id() == 19L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 19L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 17L );
        CHECK( cursor.get_parent_id() == 17L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 15L );
        CHECK( cursor.get_parent_id() == 15L );
        //cursor.dump_ids();

        --cursor;
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 15L );
        CHECK( cursor.get_parent_id() == 15L );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_303)
    {
        //303 to last element if pointing to 'end-of-document' value
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 120L );
        CHECK( cursor.get_parent_id() == 120L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_304)
    {
        //304 to last element if pointing to 'end-of-document' value. Several levels
        create_document_4();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(64L);   //last score
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end );

        --cursor;
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, move_prev_305)
    {
        //305 to added element when doc was empty
        create_empty_document();
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == nullptr );

        ImoDocument* pImoDoc = m_pDoc->get_im_root();
        pImoDoc->add_paragraph();

        CHECK( *cursor == nullptr );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 103L );
        CHECK( cursor.get_parent_id() == 50L );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_clef() == true );
        CHECK( cursor.get_pointee_id() == 103L );
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_501)
    {
        //501. point to, by id. Positioning at content level element
        create_document_1();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(120L);   //paragraph

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 120L );
        CHECK( cursor.get_parent_id() == 120L );
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

        cursor.point_to(120L);   //paragraph

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_paragraph() == true );
        CHECK( cursor.get_pointee_id() == 120L );
        CHECK( cursor.get_parent_id() == 120L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_503)
    {
        //503. point to, by id. If not found goes to end
        create_document_1();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(175L);

        CHECK( *cursor == nullptr );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_note() == true );
        CHECK( cursor.get_pointee_id() == 106L );
        CHECK( cursor.get_parent_id() == 50L );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_key_signature() == true );
        CHECK( cursor.get_pointee_id() == 104L );
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_506)
    {
        //506. point to, by id. Positioning at non-terminal at inner level
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(64L);   //score

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
        CHECK( cursor.is_inside_terminal_node() == false );
        //cursor.dump_ids();
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_507)
    {
        //507. point to, by id. Positioning at terminal at inner level
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        cursor.point_to(71L);   //first note in score

        CHECK( *cursor != nullptr );
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
        cursor.point_to(120L);   //paragraph

        cursor.to_start();

        CHECK( cursor.get_pointee_id() == 50L );
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, to_start_602)
    {
        //602. to_start. Delegating, stops delegation and moves to first top level
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //note

        cursor.to_start();

        CHECK( cursor.get_pointee_id() == 50L );
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == false );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_701)
    {
        //701. get state. Non-terminal at first level, pointing object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;       //paragraph

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 120L );
        CHECK( state.get_delegate_state() == nullptr );
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
        CHECK( state.get_delegate_state() == nullptr );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_703)
    {
        //703. get state. Parent is first level. delegating.
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(106L);   //first note

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 50L );     //score
        CHECK( state.get_delegate_state() != nullptr );
        ScoreCursorState* pSCE = dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE && pSCE->id() == 106L );
        CHECK( pSCE && pSCE->instrument() == 0 );
        CHECK( pSCE && pSCE->measure() == 0 );
        CHECK( pSCE && pSCE->staff() == 0 );
        CHECK( is_equal_time(pSCE->time(), 0.0f) );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_704)
    {
        //704. get state. delegating. Parent is first level. At end of score
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(107L);   //last rest
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 50L );     //score
        CHECK( state.get_delegate_state() != nullptr );
        ScoreCursorState* pSCE =
            dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE && pSCE->is_at_end_of_score() );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_705)
    {
        //705. get state. Non-terminal at inner level, pointing object
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);       //paragraph inside content

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 20L );
        CHECK( state.get_delegate_state() == nullptr );
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
        CHECK( state.get_delegate_state() == nullptr );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_707)
    {
        //707. get state. Parent is inner level. delegating
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(71L);   //first note in score 64L

        DocCursorState state = cursor.get_state();

        CHECK( state.get_parent_level_id() == 64L );     //score
        CHECK( state.get_delegate_state() != nullptr );
        ScoreCursorState* pSCE = dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE && pSCE->id() == 71L );
        CHECK( pSCE && pSCE->instrument() == 0 );
        CHECK( pSCE && pSCE->measure() == 0 );
        CHECK( pSCE && pSCE->staff() == 0 );
        CHECK( pSCE && is_equal_time(pSCE->time(), 0.0f) );
    }

    TEST_FIXTURE(DocCursorTestFixture, get_state_708)
    {
        //708. get state. Parent is inner level. delegating. At end of score
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(101L);   //last barline in score 64L
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );

        DocCursorState state = cursor.get_state();
        //cout << cursor.dump_cursor() << endl;
        //cursor.dump_ids();

        CHECK( state.get_parent_level_id() == 64L );     //score
        CHECK( state.get_delegate_state() != nullptr );
        ScoreCursorState* pSCE =
            dynamic_cast<ScoreCursorState*>( state.get_delegate_state().get() );
        CHECK( pSCE && pSCE->is_at_end_of_score() );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_720)
    {
        //720. restore_state. Non-terminal at first level, pointing object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;       //paragraph 108L
        DocCursorState state = cursor.get_state();
        CHECK( state.get_parent_level_id() == 120L );
        CHECK( state.get_delegate_state() == nullptr );

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 120L );
        CHECK( cursor.get_parent_id() == 120L );
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
        CHECK( cursor.get_parent_id() == 50L );
        CHECK( cursor.is_inside_terminal_node() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_722)
    {
        //722. restore_state. Non-terminal at inner level, pointing object
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(20L);       //paragraph 12L inside content 10L
        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 19L );
        DocCursorState state = cursor.get_state();
        CHECK( state.get_parent_level_id() == 20L );
        CHECK( state.get_delegate_state() == nullptr );

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( cursor.get_pointee_id() == 20L );
        CHECK( cursor.get_parent_id() == 19L );
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
        CHECK( state.get_delegate_state() == nullptr );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_725)
    {
        //725. restore state. Parent is first level. delegating. At end of score
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(107L);   //last rest
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 50L );
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 50L );
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
        CHECK( state.get_delegate_state() == nullptr );
    }

    TEST_FIXTURE(DocCursorTestFixture, restore_state_727)
    {
        //727. restore state. Parent is inner level. delegating. At end of score
        create_document_3();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(101L);   //last barline in score 64
        ++cursor;
        CHECK( *cursor == nullptr );
        CHECK( cursor.get_pointee_id() == k_cursor_at_end_of_child );
        CHECK( cursor.get_parent_id() == 64L );
        DocCursorState state = cursor.get_state();

        cursor.to_start();      //move to antoher place
        cursor.restore_state(state);

        CHECK( *cursor == nullptr );
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
        delete pScore;

        cursor.reset_and_point_after(id);

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor != nullptr );
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
        delete pImo;

        cursor.reset_and_point_after(id);     //should move to end

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor == nullptr );
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
        delete pImo;

        cursor.reset_and_point_after(id);

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor != nullptr );
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
        delete pImo;

        cursor.reset_and_point_after(id);     //should move to end

        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( *cursor == nullptr );
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
        cursor.point_to(21L);   //paragraph
        ImoId id = cursor.find_previous_pos_state().get_parent_level_id();

        ImoParagraph* pImo = static_cast<ImoParagraph*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);
        delete pImo;

        cursor.reset_and_point_after(id);     //should move to paragraph 15

        CHECK( (*cursor)->is_paragraph() );
        CHECK( cursor.is_inside_terminal_node() == false );
        CHECK( cursor.get_pointee_id() == 24L );
        CHECK( cursor.get_parent_id() == 23L );

        ++cursor;
        CHECK( (*cursor)->is_score() );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
    }

    TEST_FIXTURE(DocCursorTestFixture, jailed_mode_820)
    {
        //820. when jailed mode, enters element and moves to first child
        create_document_3();
        MyDocCursor cursor(m_pDoc);

        bool fJailed = cursor.jailed_mode_in(64L);    //score

        CHECK( *cursor != nullptr );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_heading() == true );
        CHECK( cursor.get_pointee_id() == 15L );
        CHECK( cursor.get_parent_id() == 15L );
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

        CHECK( *cursor != nullptr );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_score() == true );
        CHECK( cursor.get_pointee_id() == 64L );
        CHECK( cursor.get_parent_id() == 23L );
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

        CHECK( *cursor != nullptr );
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

        CHECK( *cursor != nullptr );
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

        CHECK( *cursor != nullptr );
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

        CHECK( *cursor != nullptr );
        CHECK( (*cursor)->is_barline() == true );
        CHECK( cursor.get_pointee_id() == 101L );
        CHECK( cursor.get_parent_id() == 64L );
        CHECK( cursor.is_inside_terminal_node() == true );
        CHECK( cursor.my_is_jailed() == true );
        //cursor.dump_ids();
    }

}

