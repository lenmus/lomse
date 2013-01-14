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
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//=======================================================================================
// DocContentCursor tests
//=======================================================================================
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
    }

    void create_document_1()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
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
        //@ initialy points to first element in content
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, empty_doc_points_to_end_of_content)
    {
        //@ if document is empty points to 'end-of-collection' value (NULL)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        DocContentCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, next)
    {
        //@ next: advances until end of content
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        ++cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, next_remains_at_end)
    {
        //@ next: when at end of content remains there
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        ++cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev)
    {
        //@ prev: moves back to previous
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_before_first)
    {
        //@ prev: when at start moves to end of content
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        --cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_from_end)
    {
        //@ prev: when at end, moves to last element
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_from_end_in_empty_doc)
    {
        //@ prev: when at end, in empty doc remains at end
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        DocContentCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id)
    {
        //@ point to, by id: goes to element if exists and it is top level
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        //cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(25L);
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_found)
    {
        //@ point to, by id: if not found go to end
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        cursor.point_to(75L);
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_id_not_top_level)
    {
        //@ point to, by id: if not top level go to end
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        cursor.point_to(23L);
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, direct_positioning_by_ptr)
    {
        //@ point to, by ptr: goes to element
        create_document_1();
        DocContentCursor cursor(m_pDoc);
        cursor.point_to(25L);
        ImoBlockLevelObj* pImo = static_cast<ImoBlockLevelObj*>( *cursor );
        cursor.point_to(100L);  //to end
        CHECK( *cursor == NULL );
        cursor.point_to(pImo);
        CHECK( *cursor != NULL );
        CHECK( *cursor == pImo );
    }

    TEST_FIXTURE(DocContentCursorTestFixture, prev_when_added_object_to_empty_doc)
    {
        //@ prev: ok when adding element to empty doc
        create_empty_document();
        DocContentCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        ImoDocument* pImoDoc = m_pDoc->get_imodoc();
        pImoDoc->add_paragraph();
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
    }

};



//=======================================================================================
// DocCursor tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// DocCursor is a cursor to traverse the document, Traversing path is based on
// user expectations when traversing visually a document.
// It is a facade object that only manages top level navigation and delegates
// on specific ElementCursors classes for traversing inner nodes.
//
// User stories:
//
// + initialy points to first element in content
// + if document is empty points to 'end-of-collection' value (NULL)
//
// - next():
//     + if traversal not delegated (implies it is pointing to a top level element):
//        + advance to next top level element if pointing to a top level element, or
//        + advance to 'end-of-collection' value if no more top level elements.
//
//   - if traversal delegated:
//        - if currently at 'end-of-child', stop delegation and advance to next
//          top level element.
//        - else, delegate and check returned value:
//          - if 'end-of-collection' (implies that the delegate was pointing to
//            last sub-element of traversed top level element): what to do?
//            caret must be placed after last subelement (i.e. at end of a score,
//            so that the user can add more notes). Let's try returning an
//            'end-of-child' value.
//          - else: do nothing. the delegate have done its job and has advanced
//  		  to next sub-element in logical sequence.
//
// - prev().
//    + if traversal not delegated:
//        + remains at first top level element if pointing to first top level element.
//        + moves back to previous top level element if pointing to a top level element
//        + moves to last top level element if pointing to 'end-of-collection' value.
//
//    - if traversal delegated:
//        - moves back to the top level element if pointing to its first sub-element.
//        - moves back to previous sub-element in logical sequence if inside a top
//          level element.
//        - moves back to previous sub-element in logical sequence if pointing
//          to 'end-of-child' value.
//
// - enter().
//    - Does nothing if pointing to a sub-element.
//    - if pointing to a top level element, delegates to specialized cursor (it will move to its initial position).
//
// - any other operation:
//    - if pointing to top level: do nothing
//    - else: delegate.
//---------------------------------------------------------------------------------------

//derivate class to have access to some protected methods
class MyDocCursor : public DocCursor
{
public:
    MyDocCursor(Document* pDoc) : DocCursor(pDoc) {}

    //access to some protected methods
    inline bool my_is_delegating() { return is_delegating(); }
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
    }

    void create_document_1()
    {
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content "
            "(score (vers 1.6) "
                "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q) )))"
            "(para (txt \"Hello world!\"))"
            "))" );
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
    TEST_FIXTURE(DocCursorTestFixture, DocCursorPointsStartOfContent)
    {
        //@ initialy points to first element in content
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        CHECK( (*cursor)->is_score() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, empty_doc_points_to_end_of_content)
    {
        //@ if document is empty points to 'end-of-collection' value (NULL)
        m_pDoc = LOMSE_NEW Document(m_libraryScope);
        m_pDoc->from_string("(lenmusdoc (vers 0.0) (content ))" );
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, next)
    {
        //@ next at top level: traverses top level and remains at end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        ++cursor;
        CHECK( *cursor == NULL );
        ++cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, prev_at_top_level)
    {
        //@ prev: when at top level traverses top level until end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        CHECK( (*cursor)->is_paragraph() == true );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
        --cursor;
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, prev_at_end)
    {
        //@ prev: to last element if pointing to 'end-of-collection' value
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ++cursor;
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, prev_when_added_object_to_empty_doc)
    {
        //@ prev: to added element when doc was empty
        create_empty_document();
        MyDocCursor cursor(m_pDoc);
        CHECK( *cursor == NULL );
        ImoDocument* pImoDoc = m_pDoc->get_imodoc();
        pImoDoc->add_paragraph();
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, enter_top_delegates)
    {
        //@enter: if pointing to a top level element, delegates
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        CHECK( !cursor.my_is_delegating() );
        cursor.enter_element();
        CHECK( cursor.my_is_delegating() );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_content_element)
    {
        //@ point to, by id. Positioning at content level element
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        //cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(25L);
        CHECK( cursor.my_is_delegating() == false );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_content_element_2)
    {
        //@ point to, by id. Removes delegation
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.enter_element();         //enter score
        CHECK( cursor.my_is_delegating() == true );
        CHECK( (*cursor)->is_clef() == true );

        cursor.point_to(25L);
        CHECK( cursor.my_is_delegating() == false );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_not_found)
    {
        //@ point to, by id. If not found goes to end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(125L);
        CHECK( cursor.my_is_delegating() == false );
        CHECK( *cursor == NULL );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_inner_element)
    {
        //@ point to, by id. Positioning at inner element
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.point_to(24L);
        CHECK( cursor.my_is_delegating() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_note() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, point_to_inner_element_2)
    {
        //@ point to, by id. moving to a sibbling
        create_document_1();
        MyDocCursor cursor(m_pDoc);
//        cout << m_pDoc->to_string(k_save_ids) << endl;
        cursor.point_to(24L);
        CHECK( (*cursor)->is_note() == true );
        cursor.point_to(22L);
        CHECK( cursor.my_is_delegating() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_key_signature() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, EnterOtherDoesNothing)
    {
        //@ enter: attempt to enter non-top element does nothing
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        cursor.enter_element();     //enter score: points to clef
        CHECK( cursor.my_is_delegating() == true );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_clef() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, reposition_after_delete_top)
    {
        //@ when top level pointed object is deleted, go to next top level object
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ImoScore* pScore = static_cast<ImoScore*>( *cursor );
        ImoDocument* pImoDoc = pScore->get_document();
        pImoDoc->erase(pScore);

        cursor.update_after_deletion();

        CHECK( cursor.my_is_delegating() == false );
        CHECK( *cursor != NULL );
        CHECK( (*cursor)->is_paragraph() == true );
    }

    TEST_FIXTURE(DocCursorTestFixture, reposition_after_delete_last_top)
    {
        //@ when last top level pointed object is deleted, go to end
        create_document_1();
        MyDocCursor cursor(m_pDoc);
        ++cursor;
        ImoParagraph* pImo = static_cast<ImoParagraph*>( *cursor );
        ImoDocument* pImoDoc = pImo->get_document();
        pImoDoc->erase(pImo);

        cursor.update_after_deletion();

        CHECK( cursor.my_is_delegating() == false );
        CHECK( *cursor == NULL );
        --cursor;
        CHECK( (*cursor)->is_score() == true );
    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorAtEndOfChild)
//    {
//        //@ next: moves back to last top level if at 'end-of-child' value
//        create_document_1();
//        MyDocCursor cursor(m_pDoc);
//        cursor.point_to(24L);       //note, last element in score
//        ++cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( cursor.is_at_end_of_child() );
//    }

//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevAtStartOfSubelement)
//    {
//        moves back to the top level element if pointing to its first sub-element
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        CHECK( (*cursor)->to_string() == "(n c4 q)" );
//        --cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevBehaviourDelegated)
//    {
//        moves back to previous sub-element in logical sequence if inside a top level element
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(n c4 q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorNextAfterEndOfChild)
//    {
//        if currently at 'end-of-child', stop delegation and advance to next
//        top level element.
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        ++cursor;   //here at end of child
//        ++cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevFromEndOfChild)
//    {
//        moves back to previous sub-element in logical sequence if pointing
//        to 'end-of-child' value.
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        ++cursor;   //here at end of child
//        --cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorCopyConstructor)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        DocCursor cursor2 = cursor;
//        cout << (*cursor2)->to_string() << endl;
//        CHECK( (*cursor2)->to_string() == "(r q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorAssignmentOperator)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        DocCursor cursor2(&doc);
//        cursor2 = cursor;
//        cout << (*cursor2)->to_string() << endl;
//        CHECK( (*cursor2)->to_string() == "(r q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorGetTopLevel)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        ++cursor;
//        LdpElement* pElm = cursor.get_top_level_element();
//        cout << (*cursor)->to_string() << endl;
//        CHECK( pElm->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorGetTopLevelDelegating)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        LdpElement* pElm = cursor.get_top_level_element();
//        cout << (*cursor)->to_string() << endl;
//        CHECK( pElm->is_type(k_score) );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_PointToObject)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.point_to(9L);
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_PointToObjectNotFound)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.point_to(4L);
//        CHECK( *cursor == NULL );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_StartOfContent)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        MyDocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        cursor.start_of_content();
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_GetState)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        ++cursor;       //(text \"this is text\")
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == 9L );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_GetStateAtEndOfCollection)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        ++cursor;
//        ++cursor;
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == -1L );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_RestoreState_NotDelegating)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        ++cursor;       //(text \"this is text\")
//        DocCursorState* pState = cursor.get_state();
//        cursor.start_of_content();      //move to antoher place
//        cursor.restore(pState);
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_RestoreState_Delegating)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;   //(r q)
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        DocCursorState* pState = cursor.get_state();
//        cursor.start_of_content();      //move to antoher place
//        cursor.restore(pState);
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_RestoreStateAtEndOfCollection)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        ++cursor;
//        ++cursor;
//        DocCursorState* pState = cursor.get_state();
//        cursor.start_of_content();      //move to antoher place
//        cursor.restore(pState);
//        CHECK( *cursor == NULL );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_ResetAndPointTo_NonDelegating)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        cursor.point_to(3L);
//        cursor.reset_and_point_to(9L);
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_ResetAndPointTo_InvokesDelegate)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        cursor.reset_and_point_to(7L);
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(n c4 q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_GetMusicData)
//    {
//        Document doc(m_libraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(m_pDoc);
//        cursor.enter_element();
//        ++cursor;       //(r q)
//        LdpElement* pElm = cursor.get_musicData_for_current_instrument();
//        cout << pElm->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        CHECK( pElm->to_string() == "(musicData (n c4 q) (r q))" );
//    }

    //Minimum test cases for any DocCursor method
    //-------------------------------------------------
    //  non delegating behaviour (one or more tests)
    //  when delegating, the right method in delegate is invoked


}

