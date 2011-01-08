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

#ifdef _LM_DEBUG_

#include <UnitTest++.h>
#include <iostream>

//classes related to these tests
#include "lomse_injectors.h"
#include "lomse_parser.h"
#include "lomse_analyser.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


class LinkerTestFixture
{
public:

    LinkerTestFixture()     //SetUp fixture
    {
        m_pLibraryScope = new LibraryScope(cout);
    }

    ~LinkerTestFixture()    //TearDown fixture
    {
        delete m_pLibraryScope;
    }

    LibraryScope* m_pLibraryScope;
};

SUITE(LinkerTest)
{

    TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMorePresentOne)
    {
        stringstream errormsg;
        LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
        stringstream expected;
        //expected << "" << endl;
        SpLdpTree tree = parser.parse_text("(score (vers 1.6)(instrument (musicData)))");
        Analyser a(errormsg, m_pLibraryScope->ldp_factory());
        a.analyse_tree(tree);
        //cout << "[" << errormsg.str() << "]" << endl;
        //cout << "[" << expected.str() << "]" << endl;
        CHECK( errormsg.str() == expected.str() );
        ImScore* pScore = dynamic_cast<ImScore*>( tree->get_root()->get_imobj() );
        CHECK( pScore != NULL );
        //CHECK( pScore->get_num_instruments() == 1 );
        delete tree->get_root();
    }

    //TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMorePresentMore)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(score (vers 1.6)(instrument (musicData))(instrument (musicData)))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImScore* pScore = dynamic_cast<ImScore*>( tree->get_root()->get_imobj() );
    //    CHECK( pScore != NULL );
    //    //CHECK( pScore->get_num_instruments() == 2 );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMoreOptionalAlternativesError)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Element 'instrument' unknown or not possible here. Removed." << endl;
    //    SpLdpTree tree = parser.parse_text("(musicData (n c4 q)(instrument 3))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    CHECK( tree->get_root()->to_string() == "(musicData (n c4 q))" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMoreOptionalAlternativesErrorRemoved)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Element 'instrument' unknown or not possible here. Removed." << endl;
    //    SpLdpTree tree = parser.parse_text("(musicData (n c4 q)(instrument 3)(n d4 e))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    CHECK( tree->get_root()->to_string() == "(musicData (n c4 q) (n d4 e))" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, AnalyserOneOrMoreOptionalAlternativesTwo)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(musicData (n c4 q)(n d4 e.))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    delete tree->get_root();
    //}

    //// note ---------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(n +d3 e.)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::Sharp );
    //    CHECK( pNote->get_dots() == 1 );
    //    CHECK( pNote->get_note_type() == ImNote::k_eighth );
    //    CHECK( pNote->get_octave() == 3 );
    //    CHECK( pNote->get_step() == ImNote::D );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_PitchError)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Unknown note pitch 'j17'. Replaced by 'c4'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n j17 q)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_quarter );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( tree->get_root()->to_string() == "(n c4 q)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_DurationErrorLetter)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Unknown note/rest duration 'j.'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 j.)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_quarter );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( tree->get_root()->to_string() == "(n c4 q)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_DurationErrorDots)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e.1)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_quarter );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( tree->get_root()->to_string() == "(n c4 q)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_Staff)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e p7)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_eighth );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( pNote->get_staff() == 6 );
    //    CHECK( tree->get_root()->to_string() == "(n c4 e p7)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_StaffError)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Invalid staff 'pz'. Replaced by 'p1'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e pz)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_staff() == 0 );
    //    CHECK( tree->get_root()->to_string() == "(n c4 e p1)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_Voice)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e v3)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_eighth );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( pNote->get_voice() == 3 );
    //    CHECK( tree->get_root()->to_string() == "(n c4 e v3)" );
    //    CHECK( pNote->is_tied_next() == false );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_VoiceError)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e vx)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_voice() == 1 );
    //    CHECK( tree->get_root()->to_string() == "(n c4 e v1)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Note_Attachment)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e v3 (text \"andante\"))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_eighth );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( pNote->get_voice() == 3 );
    //    CHECK( pNote->has_attachments() == true );
    //    delete tree->get_root();
    //}

    //// tie --------------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_TieOld)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(n c4 e v3 l)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImNote* pNote = dynamic_cast<ImNote*>( tree->get_root()->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_accidentals() == ImNote::NoAccidentals );
    //    CHECK( pNote->get_dots() == 0 );
    //    CHECK( pNote->get_note_type() == ImNote::k_eighth );
    //    CHECK( pNote->get_octave() == 4 );
    //    CHECK( pNote->get_step() == ImNote::C );
    //    CHECK( pNote->get_voice() == 3 );
    //    CHECK( pNote->is_tied_next() == true );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Tie_ParsedStop)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(tie 12 stop)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImTie* pTie = dynamic_cast<ImTie*>( tree->get_root()->get_imobj() );
    //    CHECK( pTie != NULL );
    //    CHECK( pTie->is_start() == false );
    //    CHECK( pTie->get_tie_number() == 12 );
    //    CHECK( pTie->get_start_note() == NULL );
    //    CHECK( pTie->get_stop_note() == NULL );
    //    CHECK( pTie->get_start_bezier() == NULL );
    //    CHECK( pTie->get_stop_bezier() == NULL );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Tie_ParsedStart)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(tie 15 start)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImTie* pTie = dynamic_cast<ImTie*>( tree->get_root()->get_imobj() );
    //    CHECK( pTie != NULL );
    //    CHECK( pTie->is_start() == true );
    //    CHECK( pTie->get_tie_number() == 15 );
    //    CHECK( pTie->get_start_note() == NULL );
    //    CHECK( pTie->get_stop_note() == NULL );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Tie_Bezier)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(tie 15 start (bezier (ctrol2-x -25)(start-y 36.765)) )");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImTie* pTie = dynamic_cast<ImTie*>( tree->get_root()->get_imobj() );
    //    CHECK( pTie != NULL );
    //    CHECK( pTie->is_start() == true );
    //    CHECK( pTie->get_tie_number() == 15 );
    //    CHECK( pTie->get_start_note() == NULL );
    //    CHECK( pTie->get_stop_note() == NULL );
    //    CHECK( pTie->get_stop_bezier() == NULL );
    //    ImBezier* pBezier = pTie->get_start_bezier();
    //    CHECK( pBezier != NULL );
    //    CHECK( pBezier->get_point(ImBezier::k_start).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_start).y == 36.765f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).x == -25.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).y == 0.0f );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Tie_ParsedError)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Missing or invalid tie type. Tie ignored." << endl;
    //    SpLdpTree tree = parser.parse_text("(tie 15 end)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    CHECK( tree->get_root() == NULL );
    //}

    //// bezier ----------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Bezier_Ok)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Unknown note/rest duration 'e.1'. Replaced by 'q'." << endl;
    //    SpLdpTree tree = parser.parse_text("(bezier ctrol1-x:-25 (start-x 36.765) ctrol1-y:55)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImBezier* pBezier = dynamic_cast<ImBezier*>( tree->get_root()->get_imobj() );
    //    CHECK( pBezier != NULL );
    //    //cout << "start.x = " << pBezier->get_point(ImBezier::k_start).x << endl;
    //    //cout << "start.y = " << pBezier->get_point(ImBezier::k_start).y << endl;
    //    //cout << "end.x = " << pBezier->get_point(ImBezier::k_end).x << endl;
    //    //cout << "end.y = " << pBezier->get_point(ImBezier::k_end).y << endl;
    //    //cout << "ctrol1.x = " << pBezier->get_point(ImBezier::k_ctrol1).x << endl;
    //    //cout << "ctrol1.y = " << pBezier->get_point(ImBezier::k_ctrol1).y << endl;
    //    //cout << "ctrol2.x = " << pBezier->get_point(ImBezier::k_ctrol2).x << endl;
    //    //cout << "ctrol2.y = " << pBezier->get_point(ImBezier::k_ctrol2).y << endl;
    //    CHECK( pBezier->get_point(ImBezier::k_start).x == 36.765f );
    //    CHECK( pBezier->get_point(ImBezier::k_start).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).x == -25.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).y == 55.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).y == 0.0f );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Bezier_Error)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Unknown tag 'startx'." << endl <<
    //        "Line 0. Element 'undefined' unknown or not possible here. Removed." << endl;
    //    SpLdpTree tree = parser.parse_text("(bezier (startx 36.765) ctrol1-x:-25 ctrol1-y:55)");
    //    //cout << tree->get_root()->to_string() << endl;
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImBezier* pBezier = dynamic_cast<ImBezier*>( tree->get_root()->get_imobj() );
    //    CHECK( pBezier != NULL );
    //    //cout << "start.x = " << pBezier->get_point(ImBezier::k_start).x << endl;
    //    //cout << "start.y = " << pBezier->get_point(ImBezier::k_start).y << endl;
    //    //cout << "end.x = " << pBezier->get_point(ImBezier::k_end).x << endl;
    //    //cout << "end.y = " << pBezier->get_point(ImBezier::k_end).y << endl;
    //    //cout << "ctrol1.x = " << pBezier->get_point(ImBezier::k_ctrol1).x << endl;
    //    //cout << "ctrol1.y = " << pBezier->get_point(ImBezier::k_ctrol1).y << endl;
    //    //cout << "ctrol2.x = " << pBezier->get_point(ImBezier::k_ctrol2).x << endl;
    //    //cout << "ctrol2.y = " << pBezier->get_point(ImBezier::k_ctrol2).y << endl;
    //    CHECK( pBezier->get_point(ImBezier::k_start).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_start).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_end).y == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).x == -25.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol1).y == 55.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).x == 0.0f );
    //    CHECK( pBezier->get_point(ImBezier::k_ctrol2).y == 0.0f );
    //    delete tree->get_root();
    //}

    //// rest ------------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Rest)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(r e.)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImRest* pRest = dynamic_cast<ImRest*>( tree->get_root()->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_dots() == 1 );
    //    CHECK( pRest->get_note_type() == ImNoteRest::k_eighth );
    //    CHECK( pRest->has_attachments() == false );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Rest_StaffNum)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(r e. p2)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImRest* pRest = dynamic_cast<ImRest*>( tree->get_root()->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_dots() == 1 );
    //    CHECK( pRest->get_note_type() == ImNoteRest::k_eighth );
    //    CHECK( pRest->get_staff() == 1 );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Rest_DefaultStaffNum)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(r e.)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImRest* pRest = dynamic_cast<ImRest*>( tree->get_root()->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_dots() == 1 );
    //    CHECK( pRest->get_note_type() == ImNoteRest::k_eighth );
    //    CHECK( pRest->get_staff() == 0 );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Rest_StaffNumInherited)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(musicData (r e. p2)(n c4 q)(n d4 e p3)(r q))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    LdpTree::iterator it = tree->begin();
    //    ++it;
    //    ImRest* pRest = dynamic_cast<ImRest*>( (*it)->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_staff() == 1 );
    //    ++it;
    //    ++it;
    //    ++it;
    //    ImNote* pNote = dynamic_cast<ImNote*>( (*it)->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_staff() == 1 );
    //    ++it;
    //    ++it;
    //    ++it;
    //    pNote = dynamic_cast<ImNote*>( (*it)->get_imobj() );
    //    CHECK( pNote != NULL );
    //    CHECK( pNote->get_staff() == 2 );
    //    ++it;
    //    ++it;
    //    ++it;
    //    ++it;
    //    pRest = dynamic_cast<ImRest*>( (*it)->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_staff() == 2 );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Rest_Attachment)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(r e. (text \"andante\"))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImRest* pRest = dynamic_cast<ImRest*>( tree->get_root()->get_imobj() );
    //    CHECK( pRest != NULL );
    //    CHECK( pRest->get_dots() == 1 );
    //    CHECK( pRest->get_note_type() == ImNoteRest::k_eighth );
    //    CHECK( pRest->has_attachments() == true );
    //    delete tree->get_root();
    //}

    //// fermata ---------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Fermata)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
    //    SpLdpTree tree = parser.parse_text("(fermata below)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImFermata* pFerm = dynamic_cast<ImFermata*>( tree->get_root()->get_imobj() );
    //    CHECK( pFerm != NULL );
    //    CHECK( pFerm->get_placement() == ImFermata::k_below );
    //    //CHECK( tree->get_root()->to_string() == "(goBack start)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Fermata_ErrorPlacement)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Unknown fermata placement 'under'. Replaced by 'above'." << endl;
    //    SpLdpTree tree = parser.parse_text("(fermata under)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImFermata* pFerm = dynamic_cast<ImFermata*>( tree->get_root()->get_imobj() );
    //    CHECK( pFerm != NULL );
    //    CHECK( pFerm->get_placement() == ImFermata::k_above );
    //    CHECK( tree->get_root()->to_string() == "(fermata above)" );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Fermata_Location)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. Invalid voice 'vx'. Replaced by 'v1'." << endl;
    //    SpLdpTree tree = parser.parse_text("(fermata above (dx 70))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImFermata* pFerm = dynamic_cast<ImFermata*>( tree->get_root()->get_imobj() );
    //    CHECK( pFerm != NULL );
    //    CHECK( pFerm->get_placement() == ImFermata::k_above );
    //    CHECK( pFerm->get_user_location_x() == 70.0f );
    //    CHECK( pFerm->get_user_location_y() == 0.0f );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Fermata_ErrorMore)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    expected << "Line 0. Element 'fermata': too many parameters. Extra parameters from 'fermata' have been removed." << endl;
    //    SpLdpTree tree = parser.parse_text("(fermata above (dx 70)(fermata below))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImFermata* pFerm = dynamic_cast<ImFermata*>( tree->get_root()->get_imobj() );
    //    CHECK( pFerm != NULL );
    //    CHECK( pFerm->get_placement() == ImFermata::k_above );
    //    CHECK( pFerm->get_user_location_x() == 70.0f );
    //    CHECK( pFerm->get_user_location_y() == 0.0f );
    //    CHECK( tree->get_root()->to_string() == "(fermata above (dx 70))" );
    //    delete tree->get_root();
    //}

    //// clef ----------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Clef)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(clef G)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImClef* pClef = dynamic_cast<ImClef*>( tree->get_root()->get_imobj() );
    //    CHECK( pClef != NULL );
    //    CHECK( pClef->get_type() == ImClef::kG3 );
    //    delete tree->get_root();
    //}

    //// key -----------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, AnalyserKey)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(key G)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImKeySignature* pKeySignature = dynamic_cast<ImKeySignature*>( tree->get_root()->get_imobj() );
    //    CHECK( pKeySignature != NULL );
    //    CHECK( pKeySignature->get_key_type() == ImKeySignature::G );
    //    CHECK( pKeySignature->get_staff() == 0 );
    //    delete tree->get_root();
    //}

    //// time signature -------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_TimeSignature)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(time 6 8)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImTimeSignature* pTimeSignature = dynamic_cast<ImTimeSignature*>( tree->get_root()->get_imobj() );
    //    CHECK( pTimeSignature != NULL );
    //    CHECK( pTimeSignature->get_beats() == 6 );
    //    CHECK( pTimeSignature->get_beat_type() == 8 );
    //    delete tree->get_root();
    //}

    //// systemLayout -------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, AnalyserSystemMargins)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. systemLayout: missing mandatory element 'systemMargins'." << endl;
    //    SpLdpTree tree = parser.parse_text("(systemMargins 0 100 0 2000)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImSystemMargins* pSM = dynamic_cast<ImSystemMargins*>( tree->get_root()->get_imobj() );
    //    CHECK( pSM != NULL );
    //    CHECK( pSM->get_left_margin() == 0.0f );
    //    CHECK( pSM->get_right_margin() == 100.0f );
    //    CHECK( pSM->get_system_distance() == 0.0f );
    //    CHECK( pSM->get_top_system_distance() == 2000.0f );
    //    delete tree->get_root();
    //}

    //TEST_FIXTURE(LinkerTestFixture, AnalyserSystemLayoutOk)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "Line 0. systemLayout: missing mandatory element 'systemMargins'." << endl;
    //    SpLdpTree tree = parser.parse_text("(systemLayout other (systemMargins 0 100 0 2000))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImSystemLayout* pSL = dynamic_cast<ImSystemLayout*>( tree->get_root()->get_imobj() );
    //    CHECK( pSL != NULL );
    //    CHECK( !pSL->is_first() );
    //    delete tree->get_root();
    //}

    //// metronome ------------------------------------------------------------------

    //// opt ----------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Opt_FloatOk)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(opt Render.SpacingFactor 0.536)");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImOption* pOpt = dynamic_cast<ImOption*>( tree->get_root()->get_imobj() );
    //    CHECK( pOpt != NULL );
    //    CHECK( pOpt->get_name() == "Render.SpacingFactor" );
    //    CHECK( pOpt->get_type() == ImOption::k_number_float );
    //    CHECK( pOpt->get_float_value() == 0.536f );
    //    delete tree->get_root();
    //}

    //// spacer --------------------------------------------------------------------

    //TEST_FIXTURE(LinkerTestFixture, Analyser_Spacer_Attachment)
    //{
    //    stringstream errormsg;
    //    LdpParser parser(errormsg, m_pLibraryScope->ldp_factory());
    //    stringstream expected;
    //    //expected << "" << endl;
    //    SpLdpTree tree = parser.parse_text("(spacer 70 (text \"andante\"))");
    //    Analyser a(errormsg, m_pLibraryScope->ldp_factory());
    //    a.analyse_tree(tree);
    //    //cout << "[" << errormsg.str() << "]" << endl;
    //    //cout << "[" << expected.str() << "]" << endl;
    //    CHECK( errormsg.str() == expected.str() );
    //    ImSpacer* pSp = dynamic_cast<ImSpacer*>( tree->get_root()->get_imobj() );
    //    CHECK( pSp != NULL );
    //    CHECK( pSp->get_width() == 70.0f );
    //    CHECK( pSp->get_staff() == 0 );
    //    CHECK( pSp->has_attachments() == true );
    //    delete tree->get_root();
    //}



}

#endif  // _LM_DEBUG_

