//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>

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


//---------------------------------------------------------------------------------
// DocCursor manages top level elements and delegation. Stories:
//
// + initialy points to first element in content
//
// + next():
//     + if traversal not delegated (implies it is pointing to a top level element):
//        + advance to next top level element if pointing to a top level element, or
//        + advance to 'end-of-collection' value if no more top level elements.
//
//   + if traversal delegated:
//        + if currently at 'end-of-child', stop delegation and advance to next
//          top level element.
//        + else, delegate and check returned value:
//          + if 'end-of-collection' (implies that the delegate was pointing to
//            last sub-element of traversed top level element): what to do?
//            caret must be placed after last subelement (i.e. at end of a score,
//            so that the user can add more notes). Let's try returning an
//            'end-of-child' value.
//          + else: do nothing. the delegate have done its job and has advanced
//  		  to next sub-element in logical sequence.
//
// + prev().
//    + if traversal not delegated:
//        + remains at first top level element if pointing to first top level element.
//        + moves back to previous top level element if pointing to a top level element
//        + moves to last top level element if pointing to 'end-of-collection' value.
//
//    + if traversal delegated:
//        + moves back to the top level element if pointing to its first sub-element.
//        + moves back to previous sub-element in logical sequence if inside a top
//          level element.
//        + moves back to previous sub-element in logical sequence if pointing
//          to 'end-of-child' value.
//
// + enter().
//    + Does nothing if pointing to a sub-element.
//    + if pointing to a top level element, delegates to specialized cursor (it will move to its initial position).
//
// - any other operation:
//    - if pointing to top level: do nothing
//    - else: delegate.
//---------------------------------------------------------------------------------

//derivate class to have access to some protected methods
class TestCursor : public DocCursor
{
public:
    TestCursor(Document* pDoc) : DocCursor(pDoc) {}

    //access to some protected methods
    inline bool now_delegating() { return is_delegating(); }
};


class DocCursorTestFixture
{
public:

    DocCursorTestFixture()     //SetUp fixture
    {
        m_scores_path = "../../../../test-scores/";
        m_pLibraryScope = new LibraryScope(cout);
    }

    ~DocCursorTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
    std::string m_scores_path;
};

SUITE(DocCursorTest)
{
    TEST_FIXTURE(DocCursorTestFixture, DocCursorPointsStartOfContent)
    {
        Document doc(*m_pLibraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
        TestCursor cursor(&doc);
        //cout << (*cursor)->to_string() << endl;
        CHECK( (*cursor)->is_score() == true );
    }

//    TEST_FIXTURE(DocCursorTestFixture, DocCursorNext)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        ++cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//        ++cursor;
//        CHECK( *cursor == NULL );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorEnterTopDelegates)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        CHECK( !cursor.now_delegating() );
//        cursor.enter_element();
//        CHECK( cursor.now_delegating() );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorEnterOtherDoesNothing)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.enter_element();
//        LdpElement* pElm = *cursor;
//        cursor.enter_element();
//        CHECK( cursor.now_delegating() );
//        CHECK( pElm == *cursor );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevAtFirstTop)
//    {
//        remains at first top level element if pointing to first top level element.
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevAtIntermediateTop)
//    {
//        moves back to previous top level element if pointing to a top level element
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevAtEndOfCollection)
//    {
//        moves to last top level element if pointing to 'end-of-collection' value
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        ++cursor;
//        ++cursor;
//        CHECK( *cursor == NULL );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorPrevAtStartOfSubelement)
//    {
//        moves back to the top level element if pointing to its first sub-element
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        --cursor;
//        CHECK( (*cursor)->to_string() == "(n c4 q)" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorAtEndOfChild)
//    {
//        moves back to last top level element if pointing to 'end-of-child' value
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        ++cursor;
//        cout << (*cursor)->to_string() << endl;
//        CHECK( cursor.is_at_end_of_child() );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorNextAfterEndOfChild)
//    {
//        if currently at 'end-of-child', stop delegation and advance to next
//        top level element.
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        ++cursor;
//        LdpElement* pElm = cursor.get_top_level_element();
//        cout << (*cursor)->to_string() << endl;
//        CHECK( pElm->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursorGetTopLevelDelegating)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (language en iso-8859-1) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.point_to(9L);
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_PointToObjectNotFound)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.point_to(4L);
//        CHECK( *cursor == NULL );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_StartOfContent)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        TestCursor cursor(&doc);
//        cursor.enter_element();
//        ++cursor;
//        CHECK( (*cursor)->to_string() == "(r q)" );
//        cursor.start_of_content();
//        CHECK( (*cursor)->to_string() == "(score (vers 1.6) (instrument (musicData (n c4 q) (r q))))" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_GetState)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
//        ++cursor;       //(text \"this is text\")
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == 9L );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_GetStateAtEndOfCollection)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
//        ++cursor;
//        ++cursor;
//        DocCursorState* pState = cursor.get_state();
//        CHECK( pState->get_id() == -1L );
//        delete pState;
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_RestoreState_NotDelegating)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) (instrument (musicData (n c4 q) (r q)))) (text \"this is text\")))" );
//        DocCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
//        cursor.point_to(3L);
//        cursor.reset_and_point_to(9L);
//        cout << (*cursor)->to_string() << endl;
//        CHECK( (*cursor)->to_string() == "(text \"this is text\")" );
//    }
//
//    TEST_FIXTURE(DocCursorTestFixture, DocCursor_ResetAndPointTo_InvokesDelegate)
//    {
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
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
//        Document doc(*m_pLibraryScope);
//        doc.from_string("(lenmusdoc#0 (vers#1 0.0) (content#2 (score#3 (vers#4 1.6) (instrument#5 (musicData#6 (n#7 c4 q) (r#8 q)))) (text#9 \"this is text\")))" );
//        DocCursor cursor(&doc);
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

